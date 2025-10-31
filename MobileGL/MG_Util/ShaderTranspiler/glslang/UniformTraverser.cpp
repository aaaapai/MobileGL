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
                }
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
