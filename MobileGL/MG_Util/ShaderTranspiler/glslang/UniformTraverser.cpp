//
// Created by Swung 0x48 on 2025/7/20.
//

#include "Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    void UniformTraverser::visitSymbol(glslang::TIntermSymbol *symbol) {
        const auto &type = symbol->getType();
        if (symbol->getQualifier().isUniform()) {
            auto name = symbol->getName();
            auto qualifier = symbol->getQualifier();

            auto &uniform = uniforms.emplace_back();
            uniform.name = name;
            uniform.storageQualifier = qualifier.storage;
            uniform.layoutLocation = qualifier.layoutLocation;
            uniform.layoutBinding = qualifier.layoutBinding;
            uniform.layoutPacking = qualifier.layoutPacking;
        } else if (type.getBasicType() == glslang::EbtSampler) {
            auto &uniform = samplers.emplace_back();
            uniform.name = symbol->getName();
            uniform.sampler = type.getSampler();
        }
    }
}
}
}
