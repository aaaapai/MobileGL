// MobileGL - MobileGL/MG_Backend/DirectVulkan/Renderer/ProgramFactory.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ProgramFactory.h"

#include "MG_Util/ShaderTranspiler/Types.h"
#include <cstring>
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

    ProgramFactory::~ProgramFactory() {
        DestroyLayoutCache();
    }

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

    ProgramFactory::HashType ProgramFactory::ComputeLayoutHash(const MG_State::GLState::ProgramObject& program) const {
        XXHASH_VERIFY(XXH64_reset(m_hashState, m_config.CacheVersion));
        const auto& spirvs = program.GetGeneratedSpirv();
        for (const auto& spv : spirvs) {
            XXHASH_VERIFY(XXH64_update(m_hashState, spv.data(), spv.size() * sizeof(Uint)));
        }

        const Uint32 blockCount = static_cast<Uint32>(program.GetActiveUniformBlocksCount());
        XXHASH_VERIFY(XXH64_update(m_hashState, &blockCount, sizeof(blockCount)));
        for (Uint32 i = 0; i < blockCount; ++i) {
            const Uint32 binding = program.GetUniformBlockBinding(i);
            XXHASH_VERIFY(XXH64_update(m_hashState, &binding, sizeof(binding)));
        }
        return XXH64_digest(m_hashState);
    }

    TextureTarget ProgramFactory::UniformTypeToTextureTarget(GLenum glType) {
        switch (glType) {
        case GL_SAMPLER_1D:
        case GL_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_1D:
            return TextureTarget::Texture1D;
        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
            return TextureTarget::Texture3D;
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
            return TextureTarget::TextureCubeMap;
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
            return TextureTarget::Texture2DMultisample;
        case GL_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            return TextureTarget::TextureBuffer;
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_1D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
            return TextureTarget::Texture1DArray;
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            return TextureTarget::Texture2DArray;
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            return TextureTarget::Texture2DMultisampleArray;
        case GL_SAMPLER_2D_RECT:
        case GL_SAMPLER_2D_RECT_SHADOW:
        case GL_INT_SAMPLER_2D_RECT:
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
            return TextureTarget::TextureRectangle;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_SHADOW:
        case GL_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        default:
            return TextureTarget::Texture2D;
        }
    }

    Bool ProgramFactory::ReflectBindingKinds(const MG_State::GLState::ProgramObject& program,
                                             Vector<DescriptorBindingKind>& outKinds) const {
        outKinds.assign(m_maxBindings, DescriptorBindingKind::None);

        const auto& spirv = program.GetGeneratedSpirv();
        for (const auto& module : spirv) {
            if (module.empty()) {
                continue;
            }

            spvc_context context = nullptr;
            spvc_parsed_ir ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            if (spvc_context_create(&context) != SPVC_SUCCESS) {
                return false;
            }

            const spvc_result parseResult = spvc_context_parse_spirv(context, module.data(), module.size(), &ir);
            if (parseResult != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_result compilerResult =
                spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);
            if (compilerResult != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const auto applyBindings = [&](spvc_resource_type resourceType, DescriptorBindingKind kind) {
                const spvc_reflected_resource* list = nullptr;
                size_t count = 0;
                if (spvc_resources_get_resource_list_for_type(resources, resourceType, &list, &count) != SPVC_SUCCESS) {
                    return;
                }
                for (size_t i = 0; i < count; ++i) {
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding >= m_maxBindings) {
                        continue;
                    }
                    if (kind == DescriptorBindingKind::CombinedImageSampler) {
                        outKinds[binding] = DescriptorBindingKind::CombinedImageSampler;
                    } else if (outKinds[binding] == DescriptorBindingKind::None) {
                        outKinds[binding] = kind;
                    }
                }
            };

            applyBindings(SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, DescriptorBindingKind::UniformBufferDynamic);
            applyBindings(SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, DescriptorBindingKind::CombinedImageSampler);

            spvc_context_destroy(context);
        }

        return true;
    }

    Bool ProgramFactory::ReflectSamplerBindings(const MG_State::GLState::ProgramObject& program,
                                                VkProgramLayout& layout) const {
        layout.samplerUniformLocationByBinding.assign(m_maxBindings, -1);
        layout.samplerTextureTargetByBinding.assign(m_maxBindings, TextureTarget::Texture2D);

        const auto& spirv = program.GetGeneratedSpirv();
        for (const auto& module : spirv) {
            if (module.empty()) {
                continue;
            }

            spvc_context context = nullptr;
            spvc_parsed_ir ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            if (spvc_context_create(&context) != SPVC_SUCCESS) {
                return false;
            }
            if (spvc_context_parse_spirv(context, module.data(), module.size(), &ir) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                             &compiler) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_reflected_resource* list = nullptr;
            size_t count = 0;
            if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, &list, &count) ==
                SPVC_SUCCESS) {
                for (size_t i = 0; i < count; ++i) {
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding >= m_maxBindings) {
                        continue;
                    }

                    String uniformName = list[i].name ? list[i].name : "";
                    Int location = program.GetUniformLocation(uniformName);
                    if (location < 0) {
                        const auto arraySuffix = uniformName.find("[0]");
                        if (arraySuffix != String::npos) {
                            uniformName = uniformName.substr(0, arraySuffix);
                            location = program.GetUniformLocation(uniformName);
                        }
                    }
                    if (location < 0) {
                        continue;
                    }

                    layout.samplerUniformLocationByBinding[binding] = location;
                    layout.samplerTextureTargetByBinding[binding] =
                        UniformTypeToTextureTarget(program.GetUniformType(static_cast<Uint>(location)));
                }
            }

            spvc_context_destroy(context);
        }

        return true;
    }

    Bool ProgramFactory::ReflectGlobalUboBinding(const MG_State::GLState::ProgramObject& program,
                                                 VkProgramLayout& layout) const {
        layout.globalUboBinding = -1;

        const auto& spirv = program.GetGeneratedSpirv();
        for (const auto& module : spirv) {
            if (module.empty()) {
                continue;
            }

            spvc_context context = nullptr;
            spvc_parsed_ir ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;

            if (spvc_context_create(&context) != SPVC_SUCCESS) {
                return false;
            }
            if (spvc_context_parse_spirv(context, module.data(), module.size(), &ir) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
                                             &compiler) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }
            if (spvc_compiler_create_shader_resources(compiler, &resources) != SPVC_SUCCESS) {
                spvc_context_destroy(context);
                continue;
            }

            const spvc_reflected_resource* list = nullptr;
            size_t count = 0;
            if (spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count) ==
                SPVC_SUCCESS) {
                for (size_t i = 0; i < count; ++i) {
                    const char* name = list[i].name ? list[i].name : "";
                    if (std::strstr(name, MG_Util::ShaderTranspiler::GLOBAL_UBO_NAME) == nullptr) {
                        continue;
                    }
                    const Uint32 binding =
                        spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding);
                    if (binding < m_maxBindings) {
                        layout.globalUboBinding = static_cast<Int>(binding);
                    }
                    break;
                }
            }

            spvc_context_destroy(context);
            if (layout.globalUboBinding >= 0) {
                break;
            }
        }
        return true;
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

    const ProgramFactory::VkProgramLayout* ProgramFactory::GetOrCreateProgramLayout(
        const MG_State::GLState::ProgramObject& program) {
        const HashType hash = ComputeLayoutHash(program);
        auto it = m_layoutCache.find(hash);
        if (it != m_layoutCache.end()) {
            return &it->second;
        }

        VkProgramLayout layout{};
        layout.hash = hash;
        if (!ReflectBindingKinds(program, layout.bindingKinds)) {
            MGLOG_E("ProgramFactory::GetOrCreateProgramLayout failed: reflection failed");
            return nullptr;
        }
        if (!ReflectSamplerBindings(program, layout)) {
            MGLOG_E("ProgramFactory::GetOrCreateProgramLayout failed: sampler reflection failed");
            return nullptr;
        }
        if (!ReflectGlobalUboBinding(program, layout)) {
            MGLOG_E("ProgramFactory::GetOrCreateProgramLayout failed: global UBO reflection failed");
            return nullptr;
        }

        Vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(m_maxBindings);
        for (Uint32 binding = 0; binding < m_maxBindings; ++binding) {
            const auto kind = layout.bindingKinds[binding];
            if (kind == DescriptorBindingKind::None) {
                continue;
            }

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            layoutBinding.pImmutableSamplers = nullptr;
            if (kind == DescriptorBindingKind::UniformBufferDynamic) {
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                layout.dynamicBindings.push_back(binding);
            } else {
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            bindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.bindingCount = static_cast<Uint32>(bindings.size());
        setLayoutInfo.pBindings = bindings.data();
        VK_VERIFY(vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &layout.descriptorSetLayout),
                  "ProgramFactory::GetOrCreateProgramLayout, vkCreateDescriptorSetLayout");

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &layout.descriptorSetLayout;
        VK_VERIFY(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &layout.pipelineLayout),
                  "ProgramFactory::GetOrCreateProgramLayout, vkCreatePipelineLayout");

        auto [insertIt, _] = m_layoutCache.emplace(hash, std::move(layout));
        return &insertIt->second;
    }

    VkPipelineLayout ProgramFactory::GetOrCreatePipelineLayout(const MG_State::GLState::ProgramObject& program) {
        const auto* layout = GetOrCreateProgramLayout(program);
        return layout ? layout->pipelineLayout : VK_NULL_HANDLE;
    }

    void ProgramFactory::DestroyLayoutCache() {
        for (auto& [_, layout] : m_layoutCache) {
            if (layout.pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, layout.pipelineLayout, nullptr);
                layout.pipelineLayout = VK_NULL_HANDLE;
            }
            if (layout.descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_device, layout.descriptorSetLayout, nullptr);
                layout.descriptorSetLayout = VK_NULL_HANDLE;
            }
        }
        m_layoutCache.clear();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
