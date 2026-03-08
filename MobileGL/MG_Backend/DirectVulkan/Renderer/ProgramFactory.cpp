// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramFactory.h"

#include <spirv-tools/libspirv.h>
#include <spirv-tools/optimizer.hpp>
#include <source/opt/constants.h>
#include <source/opt/instruction.h>
#include <source/opt/ir_builder.h>
#include <source/opt/ir_context.h>
#include <source/opt/module.h>
#include <source/opt/pass.h>
#include <source/opt/type_manager.h>

namespace MobileGL::MG_Backend::DirectVulkan {
    namespace {
        using ShaderObject = MG_State::GLState::ShaderObject;

        struct PositionTargetInfo {
            Uint32 variableId = 0;
            Uint32 vectorTypeId = 0;
            Uint32 floatTypeId = 0;
            Uint32 vectorPtrTypeId = 0;
            Uint32 memberIndex = 0;
            Bool isMember = false;
        };

        Bool IsVec4Float32(spvtools::opt::IRContext* context, Uint32 typeId, Uint32* outFloatTypeId) {
            auto* vecInst = context->get_def_use_mgr()->GetDef(typeId);
            if (!vecInst || vecInst->opcode() != spv::Op::OpTypeVector) return false;
            if (vecInst->GetSingleWordInOperand(1) != 4) return false;

            const Uint32 floatTypeId = vecInst->GetSingleWordInOperand(0);
            auto* floatInst = context->get_def_use_mgr()->GetDef(floatTypeId);
            if (!floatInst || floatInst->opcode() != spv::Op::OpTypeFloat) return false;
            if (floatInst->GetSingleWordInOperand(0) != 32) return false;

            if (outFloatTypeId) *outFloatTypeId = floatTypeId;
            return true;
        }

        Bool ResolveDirectPositionTarget(spvtools::opt::IRContext* context, Uint32 variableId,
                                         PositionTargetInfo* outTarget) {
            auto* varInst = context->get_def_use_mgr()->GetDef(variableId);
            if (!varInst || varInst->opcode() != spv::Op::OpVariable) return false;
            if (varInst->GetSingleWordInOperand(0) != static_cast<Uint32>(spv::StorageClass::Output)) return false;

            auto* ptrTypeInst = context->get_def_use_mgr()->GetDef(varInst->type_id());
            if (!ptrTypeInst || ptrTypeInst->opcode() != spv::Op::OpTypePointer) return false;
            if (ptrTypeInst->GetSingleWordInOperand(0) != static_cast<Uint32>(spv::StorageClass::Output)) return false;

            PositionTargetInfo target{};
            target.variableId = variableId;
            target.vectorTypeId = ptrTypeInst->GetSingleWordInOperand(1);
            if (!IsVec4Float32(context, target.vectorTypeId, &target.floatTypeId)) return false;
            target.vectorPtrTypeId = varInst->type_id();
            target.isMember = false;

            *outTarget = target;
            return true;
        }

        Uint32 FindOutputVectorPointerTypeId(spvtools::opt::IRContext* context, Uint32 vectorTypeId) {
            auto* vectorType = context->get_type_mgr()->GetType(vectorTypeId);
            if (!vectorType) return 0;
            spvtools::opt::analysis::Pointer ptrType(vectorType, spv::StorageClass::Output);
            return context->get_type_mgr()->GetTypeInstruction(&ptrType);
        }

        Bool ResolveMemberPositionTarget(spvtools::opt::IRContext* context, Uint32 structTypeId, Uint32 memberIndex,
                                         PositionTargetInfo* outTarget) {
            auto* structInst = context->get_def_use_mgr()->GetDef(structTypeId);
            if (!structInst || structInst->opcode() != spv::Op::OpTypeStruct) return false;
            if (memberIndex >= structInst->NumInOperands()) return false;

            const Uint32 vectorTypeId = structInst->GetSingleWordInOperand(memberIndex);
            Uint32 floatTypeId = 0;
            if (!IsVec4Float32(context, vectorTypeId, &floatTypeId)) return false;

            const Uint32 vectorPtrTypeId = FindOutputVectorPointerTypeId(context, vectorTypeId);
            if (vectorPtrTypeId == 0) return false;

            for (auto& inst : context->module()->types_values()) {
                if (inst.opcode() != spv::Op::OpVariable) continue;
                if (inst.GetSingleWordInOperand(0) != static_cast<Uint32>(spv::StorageClass::Output)) continue;

                auto* ptrTypeInst = context->get_def_use_mgr()->GetDef(inst.type_id());
                if (!ptrTypeInst || ptrTypeInst->opcode() != spv::Op::OpTypePointer) continue;
                if (ptrTypeInst->GetSingleWordInOperand(0) != static_cast<Uint32>(spv::StorageClass::Output)) continue;
                if (ptrTypeInst->GetSingleWordInOperand(1) != structTypeId) continue;

                PositionTargetInfo target{};
                target.variableId = inst.result_id();
                target.vectorTypeId = vectorTypeId;
                target.floatTypeId = floatTypeId;
                target.vectorPtrTypeId = vectorPtrTypeId;
                target.memberIndex = memberIndex;
                target.isMember = true;
                *outTarget = target;
                return true;
            }

            return false;
        }

        Bool FindPositionTarget(spvtools::opt::IRContext* context, PositionTargetInfo* outTarget) {
            Vector<Pair<Uint32, Uint32>> memberCandidates;
            constexpr auto kDecorationBuiltIn = static_cast<Uint32>(spv::Decoration::BuiltIn);
            constexpr auto kBuiltInPosition = static_cast<Uint32>(spv::BuiltIn::Position);

            for (auto& inst : context->module()->annotations()) {
                if (inst.opcode() == spv::Op::OpDecorate) {
                    if (inst.NumInOperands() < 3) continue;
                    if (inst.GetSingleWordInOperand(1) != kDecorationBuiltIn) continue;
                    if (inst.GetSingleWordInOperand(2) != kBuiltInPosition) continue;
                    if (ResolveDirectPositionTarget(context, inst.GetSingleWordInOperand(0), outTarget)) return true;
                } else if (inst.opcode() == spv::Op::OpMemberDecorate) {
                    if (inst.NumInOperands() < 4) continue;
                    if (inst.GetSingleWordInOperand(2) != kDecorationBuiltIn) continue;
                    if (inst.GetSingleWordInOperand(3) != kBuiltInPosition) continue;
                    memberCandidates.emplace_back(inst.GetSingleWordInOperand(0), inst.GetSingleWordInOperand(1));
                }
            }

            for (const auto& [structTypeId, memberIndex] : memberCandidates) {
                if (ResolveMemberPositionTarget(context, structTypeId, memberIndex, outTarget)) return true;
            }
            return false;
        }

        Bool InsertPositionFixup(spvtools::opt::IRContext* context, spvtools::opt::Instruction* insertBefore,
                                 const PositionTargetInfo& target, Uint32 halfConstId, Bool doYFlip, Bool doZRemap,
                                 Bool doSurfaceRotate90, Bool doSurfaceRotate180, Bool doSurfaceRotate270) {
            using namespace spvtools::opt;
            InstructionBuilder builder(context, insertBefore,
                                       IRContext::kAnalysisDefUse | IRContext::kAnalysisInstrToBlockMapping);

            Uint32 positionPtrId = target.variableId;
            if (target.isMember) {
                const Uint32 memberIndexId = builder.GetUintConstantId(target.memberIndex);
                if (memberIndexId == 0) return false;
                auto* access = builder.AddAccessChain(target.vectorPtrTypeId, target.variableId, {memberIndexId});
                if (!access) return false;
                positionPtrId = access->result_id();
            }

            auto* position = builder.AddLoad(target.vectorTypeId, positionPtrId);
            if (!position) return false;
            auto* x = builder.AddCompositeExtract(target.floatTypeId, position->result_id(), {0});
            auto* y = builder.AddCompositeExtract(target.floatTypeId, position->result_id(), {1});
            auto* z = builder.AddCompositeExtract(target.floatTypeId, position->result_id(), {2});
            auto* w = builder.AddCompositeExtract(target.floatTypeId, position->result_id(), {3});
            if (!x || !y || !z || !w) return false;

            if (!doYFlip && !doZRemap && !doSurfaceRotate90 && !doSurfaceRotate180 && !doSurfaceRotate270) {
                return false;
            }

            Uint32 xValueId = x->result_id();
            Uint32 yValueId = y->result_id();
            if (doYFlip) {
                auto* negY = builder.AddUnaryOp(target.floatTypeId, spv::Op::OpFNegate, y->result_id());
                if (!negY) return false;
                yValueId = negY->result_id();
            }

            if (doSurfaceRotate90) {
                auto* negY = builder.AddUnaryOp(target.floatTypeId, spv::Op::OpFNegate, yValueId);
                if (!negY) return false;
                xValueId = negY->result_id();
                yValueId = x->result_id();
            } else if (doSurfaceRotate180) {
                auto* negX = builder.AddUnaryOp(target.floatTypeId, spv::Op::OpFNegate, xValueId);
                auto* negY = builder.AddUnaryOp(target.floatTypeId, spv::Op::OpFNegate, yValueId);
                if (!negX || !negY) return false;
                xValueId = negX->result_id();
                yValueId = negY->result_id();
            } else if (doSurfaceRotate270) {
                auto* negX = builder.AddUnaryOp(target.floatTypeId, spv::Op::OpFNegate, xValueId);
                if (!negX) return false;
                xValueId = yValueId;
                yValueId = negX->result_id();
            }

            Uint32 zValueId = z->result_id();
            if (doZRemap) {
                auto* zPlusW = builder.AddBinaryOp(target.floatTypeId, spv::Op::OpFAdd, z->result_id(), w->result_id());
                if (!zPlusW) return false;
                auto* mappedZ =
                    builder.AddBinaryOp(target.floatTypeId, spv::Op::OpFMul, zPlusW->result_id(), halfConstId);
                if (!mappedZ) return false;
                zValueId = mappedZ->result_id();
            }

            auto* fixedPosition = builder.AddCompositeConstruct(target.vectorTypeId,
                                                                {xValueId, yValueId, zValueId, w->result_id()});
            if (!fixedPosition) return false;

            return builder.AddStore(positionPtrId, fixedPosition->result_id()) != nullptr;
        }

        class GlToVulkanPositionFixPass final : public spvtools::opt::Pass {
        public:
            const char* name() const override { return "gl-to-vulkan-position-fix"; }
            explicit GlToVulkanPositionFixPass(ProgramFactory::CompileOptionFlags transformFlags)
                : m_transformFlags(transformFlags) {}

            Status Process() override {
                if (!m_transformFlags) return Status::SuccessWithoutChange;
                PositionTargetInfo target{};
                if (!FindPositionTarget(context(), &target)) return Status::SuccessWithoutChange;

                auto* floatType = context()->get_type_mgr()->GetType(target.floatTypeId);
                if (!floatType) return Status::SuccessWithoutChange;

                const auto halfBits = std::bit_cast<Uint32>(0.5f);
                const auto* halfConst = context()->get_constant_mgr()->GetConstant(floatType, {halfBits});
                auto* halfInst = context()->get_constant_mgr()->GetDefiningInstruction(halfConst);
                if (!halfInst) return Status::SuccessWithoutChange;
                const Uint32 halfConstId = halfInst->result_id();

                const Bool doYFlip = (m_transformFlags & ProgramFactory::CompileOptionBit::PositionYFlip);
                const Bool doZRemap = (m_transformFlags & ProgramFactory::CompileOptionBit::PositionZRemap);
                const Bool doSurfaceRotate90 = (m_transformFlags & ProgramFactory::CompileOptionBit::SurfaceRotate90);
                const Bool doSurfaceRotate180 =
                    (m_transformFlags & ProgramFactory::CompileOptionBit::SurfaceRotate180);
                const Bool doSurfaceRotate270 =
                    (m_transformFlags & ProgramFactory::CompileOptionBit::SurfaceRotate270);

                Bool modified = false;
                for (auto& entryPoint : get_module()->entry_points()) {
                    if (entryPoint.opcode() != spv::Op::OpEntryPoint) continue;
                    if (entryPoint.NumInOperands() < 2) continue;

                    const auto model = static_cast<spv::ExecutionModel>(entryPoint.GetSingleWordInOperand(0));
                    if (model != spv::ExecutionModel::Vertex && model != spv::ExecutionModel::TessellationEvaluation &&
                        model != spv::ExecutionModel::Geometry) {
                        continue;
                    }

                    auto* function = context()->GetFunction(entryPoint.GetSingleWordInOperand(1));
                    if (!function) continue;

                    for (auto& bb : *function) {
                        for (auto instIter = bb.begin(); instIter != bb.end(); ++instIter) {
                            auto* inst = &*instIter;
                            const Bool needsFixup =
                                (model == spv::ExecutionModel::Geometry && inst->opcode() == spv::Op::OpEmitVertex) ||
                                (model != spv::ExecutionModel::Geometry && inst->opcode() == spv::Op::OpReturn);
                            if (!needsFixup) continue;

                            modified |= InsertPositionFixup(context(), inst, target, halfConstId, doYFlip, doZRemap,
                                                             doSurfaceRotate90, doSurfaceRotate180, doSurfaceRotate270);
                        }
                    }
                }

                if (!modified) return Status::SuccessWithoutChange;
                context()->InvalidateAnalysesExceptFor(spvtools::opt::IRContext::kAnalysisDefUse |
                                                       spvtools::opt::IRContext::kAnalysisInstrToBlockMapping);
                return Status::SuccessWithChange;
            }

        private:
            ProgramFactory::CompileOptionFlags m_transformFlags;
        };

        spvtools::Optimizer::PassToken CreateGlToVulkanPositionFixPass(
            ProgramFactory::CompileOptionFlags transformFlags) {
            return spvtools::Optimizer::PassToken(MakeUnique<GlToVulkanPositionFixPass>(transformFlags));
        }

        Bool TransformSpirvForVulkanPositionFix(const Vector<Uint>& input, Vector<Uint>& output,
                                                ProgramFactory::CompileOptionFlags transformFlags) {
            if (input.empty()) {
                output.clear();
                return true;
            }

            if (!transformFlags) {
                output = input;
                return true;
            }

            spvtools::Optimizer optimizer(SPV_ENV_VULKAN_1_3);
            spvtools::OptimizerOptions options;
            options.set_run_validator(false);
            optimizer.RegisterPass(CreateGlToVulkanPositionFixPass(transformFlags));

            const Bool success = optimizer.Run(input.data(), input.size(), &output, options);
            if (!success) {
                MGLOG_E("Vulkan: failed to run GL->Vulkan position fix pass");
                output = input;
            }
            return success;
        }

        ShaderStage PickClipFixupStage(const Vector<SharedPtr<ShaderObject>>& shaders) {
            Bool hasGeometry = false;
            Bool hasTessEval = false;
            Bool hasVertex = false;

            for (const auto& shader : shaders) {
                if (!shader) continue;
                const auto stage = shader->GetShaderStage();
                hasGeometry |= (stage == ShaderStage::Geometry);
                hasTessEval |= (stage == ShaderStage::TessEval);
                hasVertex |= (stage == ShaderStage::Vertex);
            }

            if (hasGeometry) return ShaderStage::Geometry;
            if (hasTessEval) return ShaderStage::TessEval;
            if (hasVertex) return ShaderStage::Vertex;
            return ShaderStage::Unknown;
        }
    } // namespace

    ProgramFactory::~ProgramFactory() = default;

    VkShaderStageFlagBits ProgramFactory::ToVkStage(ShaderStage stage) {
        switch (stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TessControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TessEval:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        }
    }

    ProgramFactory::HashType ProgramFactory::ComputeHash(const MG_State::GLState::ProgramObject& program,
                                                         CompileOptionFlags flags) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        // We expect shader stages in program object are sorted
        const auto& spirvs = program.GetGeneratedSpirv();
        for (const auto& spv : spirvs) {
            XXHASH_VERIFY(XXH64_update(m_hashState, spv.data(), spv.size() * sizeof(Uint)));
        }
        XXHASH_VERIFY(XXH64_update(m_hashState, &flags, sizeof(CompileOptionFlags)));
        HashType hash = XXH64_digest(m_hashState);
        return hash;
    }

    Vector<VkPipelineShaderStageCreateInfo>& ProgramFactory::GetOrCreatePipelineShaderStages(
        const MG_State::GLState::ProgramObject& program, CompileOptionFlags flags) {
        auto hash = ComputeHash(program, flags);
        auto it = m_cache.find(hash);
        if (it != m_cache.end()) {
            return it->second.stages;
        }

        auto& entry = m_cache[hash];
        entry.hash = hash;
        auto& shaders = program.GetAttachedShaders();
        auto& spirv = program.GetGeneratedSpirv();

        const ShaderStage fixupStage = PickClipFixupStage(shaders);

        for (SizeT i = 0; i < shaders.size(); ++i) {
            auto& spv = spirv[i];
            if (spv.empty()) continue;

            Vector<Uint> moduleSpv;

            // Apply position fixup if needed
            if (fixupStage != ShaderStage::Unknown && shaders[i] && shaders[i]->GetShaderStage() == fixupStage) {
                TransformSpirvForVulkanPositionFix(spv, moduleSpv, flags);
            } else {
                moduleSpv = spv;
            }

            VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            smci.codeSize = moduleSpv.size() * sizeof(Uint);
            smci.pCode = moduleSpv.data();

            VkShaderModule module = VK_NULL_HANDLE;
            VK_VERIFY(vkCreateShaderModule(m_device, &smci, nullptr, &module), "vkCreateShaderModule");

            VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            ShaderStage shaderStage = shaders[i]->GetShaderStage();
            stage.stage = ToVkStage(shaderStage);
            stage.module = module;
            stage.pName = "main";

            entry.modules.push_back(module);
            entry.stages.push_back(stage);
        }

        return entry.stages;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
