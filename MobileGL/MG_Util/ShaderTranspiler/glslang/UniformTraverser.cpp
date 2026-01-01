// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
