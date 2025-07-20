//
// Created by Swung 0x48 on 2025/7/20.
//

#ifndef MOBILEGL_UNIFORMTRAVERSER_H
#define MOBILEGL_UNIFORMTRAVERSER_H

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    class UniformTraverser : public glslang::TIntermTraverser {
    public:
        UniformTraverser(Vector<TUniform<TUniformType::Uniform>> &u, Vector<TUniform<TUniformType::Sampler>> &s)
                : uniforms(u), samplers(s) {}

        void visitSymbol(glslang::TIntermSymbol *symbol) override;

        Vector<TUniform<TUniformType::Uniform>> &uniforms;
        Vector<TUniform<TUniformType::Sampler>> &samplers;
    };
}
}
}

#endif //MOBILEGL_UNIFORMTRAVERSER_H
