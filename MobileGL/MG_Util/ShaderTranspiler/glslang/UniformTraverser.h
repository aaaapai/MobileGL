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