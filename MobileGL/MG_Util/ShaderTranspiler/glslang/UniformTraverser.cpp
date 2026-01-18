// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
