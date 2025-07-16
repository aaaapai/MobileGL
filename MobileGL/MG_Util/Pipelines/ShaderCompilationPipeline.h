//
// Created by Swung 0x48 on 2025-07-12.
//

#ifndef MG_UTIL_PIPELINES_SHADERCOMPILATIONPIPELINE_H
#define MG_UTIL_PIPELINES_SHADERCOMPILATIONPIPELINE_H

#include "../../Includes.h"

namespace MobileGL {
namespace MG_Util {
namespace Pipeline {
    class UniformTraverser : public glslang::TIntermTraverser {
    public:
        UniformTraverser(Vector<TUniform<TUniformType::Uniform>>& u, Vector<TUniform<TUniformType::Sampler>>& s): uniforms(u), samplers(s) {}

        void visitSymbol(glslang::TIntermSymbol* symbol) override {
            const auto& type = symbol->getType();
            if (symbol->getQualifier().isUniform()) {
                auto name = symbol->getName();
                auto qualifier = symbol->getQualifier();

                auto& uniform = uniforms.emplace_back();
                uniform.name = name;
                uniform.storageQualifier = qualifier.storage;
                uniform.layoutLocation = qualifier.layoutLocation;
                uniform.layoutBinding = qualifier.layoutBinding;
                uniform.layoutPacking = qualifier.layoutPacking;
            } else if (type.getBasicType() == glslang::EbtSampler) {
                auto& uniform = samplers.emplace_back();
                uniform.name = symbol->getName();
                uniform.sampler = type.getSampler();
            }
        }

        Vector<TUniform<TUniformType::Uniform>>& uniforms;
        Vector<TUniform<TUniformType::Sampler>>& samplers;
    };

    class ShaderCompilationPipeline: public IPipeline<ShaderCompilationPipeline, ShaderPayload> {
        explicit ShaderCompilationPipeline(std::function<void(ShaderPayload &)> callback);
        void Invoke(SharedPtr<ShaderPayload> payload);

        static TBuiltInResource& GetTBuiltInResourceInstance();
    private:
        PipelineExecutor<ShaderPayload> executor_;
    };
}
}
}


#endif //MG_UTIL_PIPELINES_SHADERCOMPILATIONPIPELINE_H
