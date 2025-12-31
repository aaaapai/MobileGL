// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/UniformTraverser.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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