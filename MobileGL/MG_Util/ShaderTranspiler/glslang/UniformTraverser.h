//
// Created by Swung 0x48 on 2025/7/20.
//

#ifndef MOBILEGL_UNIFORMTRAVERSER_H
#define MOBILEGL_UNIFORMTRAVERSER_H

#include "Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    class UniformTraverser : public glslang::TIntermTraverser {
    public:
        void visitSymbol(glslang::TIntermSymbol *symbol) override;
        Vector<glslang::TIntermSymbol*>& GetCollectedSymbols() { return m_collectedSymbols; }
    private:
        Vector<glslang::TIntermSymbol*> m_collectedSymbols;
    };
}
}
}

#endif //MOBILEGL_UNIFORMTRAVERSER_H
