#include "ShaderObject.h"
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>

#include "MG_Util/ShaderTranspiler/glslang/UniformTraverser.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ShaderObject::SetShaderSource(const String& source) {
                m_source = source;
            }

            void ShaderObject::SetShaderSource(String &&source) {
                m_source = Move(source);
            }

            void ShaderObject::Compile() {
                // if (!DoReflection()) {
                //     return;
                // }

                using namespace MG_Util::ShaderTranspiler;

                // Compile for OpenGL here, so that we can do validation and link
                // like a real OpenGL driver at linking stage
                // Will compile for other backends later.
                ShaderAttrib attrib{
                    .shaderType = GetGLShaderTypeByMGLShaderStage(m_stage),
                    .sourceStr = m_source,
                    .flags = ShaderCompileBits::CompileForOpenGL
                };

                auto result = ShaderCompiler::CompileShader(attrib);
                if (result) {
                    m_compileStatus = true;
                    m_shader = result.value();
                } else {
                    m_compileStatus = false;
                    m_infoLog = result.error().log;
                }
            }

            void ShaderObject::MarkAsDeleted() {
                m_deleteStatus = true;
            }

            // bool ShaderObject::DoReflection() {
            //     using namespace MG_Util::ShaderTranspiler;
            //     ShaderAttrib attrib{
            //         .shaderType = GetGLShaderTypeByMGLShaderStage(m_stage),
            //         .sourceStr = m_source,
            //         .flags = ShaderCompileBits::CompileForOpenGL
            //     };
            //
            //     auto result = ShaderCompiler::CompileShader(attrib);
            //     if (!result) {
            //         m_compileStatus = false;
            //         m_infoLog = result.error().log;
            //
            //         const std::string e = std::format("Shader compilation failed: \nerrc: {}\nmsg: {}\n",
            //                                           result.error().errc, result.error().log);
            //         return false;
            //     }
            //
            //     auto pShader = result.value();
            //     auto root = pShader->getIntermediate()->getTreeRoot();
            //     UniformTraverser traverser;
            //     root->traverse(&traverser);
            //     auto& symbols = traverser.GetCollectedSymbols();
            //     for (const auto& symbol : symbols) {
            //         m_uniforms[symbol->getName().c_str()] = symbol->getQualifier().layoutLocation;
            //     }
            //
            //     return true;
            // }

        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL