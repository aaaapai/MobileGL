// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "Includes.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            class UniformTraverser : public glslang::TIntermTraverser {
            public:
                void visitSymbol(glslang::TIntermSymbol* symbol) override;
                Vector<glslang::TIntermSymbol*>& GetCollectedSymbols() { return m_collectedSymbols; }

            private:
                Vector<glslang::TIntermSymbol*> m_collectedSymbols;
            };
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL