//
// Created by Swung 0x48 on 2025/7/20.
//

#include "UniformTraverser.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            void UniformTraverser::visitSymbol(glslang::TIntermSymbol* symbol) {
                auto parent = getParentNode();
                if (!parent) return;
                auto parentAgg = parent->getAsAggregate();
                if (!parentAgg || parentAgg->getOp() != glslang::EOpLinkerObjects) return;

                const auto& type = symbol->getType();
                if (symbol->getQualifier().isUniform() && type.getBasicType() != glslang::EbtSampler) {
                    m_collectedSymbols.emplace_back(symbol);
                    // auto& name = symbol->getName();
                    // auto qualifier = symbol->getQualifier();
                    //
                    // printf("layout(location = %d) uniform %s\n", qualifier.layoutLocation, name.c_str());
                    // auto &uniform = uniforms.emplace_back();
                    // uniform.name = name;
                    // uniform.storageQualifier = qualifier.storage;
                    // uniform.layoutLocation = qualifier.layoutLocation;
                    // uniform.layoutBinding = qualifier.layoutBinding;
                    // uniform.layoutPacking = qualifier.layoutPacking;
                }
                // else if (type.getBasicType() == glslang::EbtSampler) {
                // auto &uniform = samplers.emplace_back();
                // uniform.name = symbol->getName();
                // uniform.sampler = type.getSampler();
                // }
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
