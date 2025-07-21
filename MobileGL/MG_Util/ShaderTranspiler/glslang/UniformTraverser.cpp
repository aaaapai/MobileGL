//
// Created by Swung 0x48 on 2025/7/20.
//

#include "Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    void UniformTraverser::visitSymbol(glslang::TIntermSymbol *symbol) {
        auto parent = getParentNode();
        if (!parent)
            return;
        auto parentAgg = parent->getAsAggregate();
        if (!parentAgg || parentAgg->getOp() != glslang::EOpLinkerObjects)
            return;

        const auto &type = symbol->getType();
        if (type.getBasicType() == glslang::EbtSampler) {
            auto &uniform = samplers.emplace_back();
            uniform.name = symbol->getName();
            uniform.sampler = type.getSampler();
        } else if (symbol->getQualifier().isUniform()) {
            auto& name = symbol->getName();
            auto qualifier = symbol->getQualifier();

            auto &uniform = uniforms.emplace_back();
            uniform.name = name;
            uniform.storageQualifier = qualifier.storage;
            uniform.layoutLocation = qualifier.layoutLocation;
            uniform.layoutBinding = qualifier.layoutBinding;
            uniform.layoutPacking = qualifier.layoutPacking;
        }
        // if (symbol->getQualifier().isUniformOrBuffer()) {
        //     auto name = symbol->getName();
        //     auto qualifier = symbol->getQualifier();
        //
        //     fprintf(stderr, "%s: %s\n", name.c_str(),
        //             glslang::GetStorageQualifierString(qualifier.storage));
        //
        //     if (qualifier.hasLocation()) {
        //         fprintf(stderr, "  loc: %d\n", qualifier.layoutLocation);
        //     }
        //     if (qualifier.hasBinding()) {
        //         fprintf(stderr, "  binding: %d\n", qualifier.layoutBinding);
        //     }
        //     if (qualifier.hasPacking()) {
        //         fprintf(stderr, "  packing: %s\n",
        //                 glslang::TQualifier::getLayoutPackingString(
        //                     qualifier.layoutPacking));
        //     }
        // }
    }
}
}
}
