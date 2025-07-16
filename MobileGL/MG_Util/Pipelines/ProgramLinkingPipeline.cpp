//
// Created by Swung 0x48 on 2025/7/16.
//

#include "ProgramLinkingPipeline.h"

namespace MobileGL {
namespace MG_Util {
namespace Pipeline {
    ProgramLinkingPipeline::ProgramLinkingPipeline(std::function<void(ProgramPayload &)> callback):
            executor_(Move(callback)) {
        executor_
            .Register([](SharedPtr<ProgramPayload> payload, auto next) {
                // Link TShader to TProgram
                auto& shaders = payload->shadersToLink;
                auto& program = payload->linkedProgram;

                for (auto& shader : shaders) {
                    program.addShader(shader.get());
                }

                if (!program.link(EShMsgDefault)) {
                    payload->log += "Error: [glslang] Cannot link the program:\n" + std::to_string(program.getInfoLog());
                    payload->errc = -1;

                    return;
                }

                (*next)();
            })
            .Register([](auto payload, auto next) {
                auto& types = payload->shaderTypes;
                auto& program = payload->linkedProgram;
                auto& programSpirv = payload->programSpirv;

                glslang::SpvOptions spvOptions;
                spvOptions.disableOptimizer = false;

                for (auto type : types) {
                    std::vector<unsigned> spirv;

                    auto lang = GetEShLanguageByShaderType(type);
                    GlslangToSpv(*program.getIntermediate(lang), spirv, &spvOptions);
                    programSpirv.emplace_back(std::move(spirv));
                }

                (*next)();
            })
            ;
    }

    void ProgramLinkingPipeline::Invoke(SharedPtr<ProgramPayload> payload) {
        executor_.Process(Move(payload));
    }
}
}
}