#include "ShaderCompiler.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {

            Result<Vector<Vector<unsigned>>> ShaderCompiler::GetSpirvBinaryFromProgram(
                const ProgramBinaryAttrib& attrib) {
                
                if (!attrib.program || attrib.shaderTypes.empty()) {
                    ResultInfo r;
                    r.log += "Invalid program or shader types\n";
                    r.errc = -1;
                    return std::unexpected(r);
                }

                glslang::SpvOptions spvOptions;
                spvOptions.disableOptimizer = false;

                Vector<Vector<unsigned>> allSpirv;
                
                try {
                    for (auto type : attrib.shaderTypes) {
                        auto* intermediate = attrib.program.getIntermediate(ConvertGLEnumToEShLanguage(type));
                        if (!intermediate) {
                            ResultInfo r;
                            r.log += "Failed to get intermediate for shader type: " + std::to_string(type) + "\n";
                            r.errc = -2;
                            return std::unexpected(r);
                        }

                        Vector<unsigned> spirv;
                        GlslangToSpv(*intermediate, spirv, &spvOptions);
                        
                        if (spirv.empty()) {
                            ResultInfo r;
                            r.log += "Generated empty SPIR-V for shader type: " + std::to_string(type) + "\n";
                            r.errc = -3;
                            return std::unexpected(r);
                        }
                        
                        allSpirv.push_back(std::move(spirv));
                    }
                } catch (const std::exception& e) {
                    ResultInfo r;
                    r.log += "Exception during SPIR-V generation: " + std::string(e.what()) + "\n";
                    r.errc = -4;
                    return std::unexpected(r);
                }

                return allSpirv;
            }

            Result<String> ShaderCompiler::DecompileShader(SpvcSession& session) {
                if (!session.IsValid()) {
                    ResultInfo r;
                    r.log += "Invalid SPIRV-Cross session\n";
                    r.errc = -1;
                    return std::unexpected(r);
                }

                spvc_compiler_options options = nullptr;
                auto result = session.CreateOptions(&options);
                if (result != SPVC_SUCCESS || !options) {
                    ResultInfo r;
                    r.log += "Failed to create compiler options: " + std::to_string(result) + "\n";
                    r.errc = -2;
                    return std::unexpected(r);
                }

                spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 320);
                spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_TRUE);
                
                result = session.SetOptions(options);
                if (result != SPVC_SUCCESS) {
                    ResultInfo r;
                    r.log += "Failed to set compiler options: " + std::to_string(result) + "\n";
                    r.errc = -3;
                    return std::unexpected(r);
                }

                const char* glslResult = nullptr;
                result = session.Compile(&glslResult);
                
                if (result != SPVC_SUCCESS || !glslResult) {
                    ResultInfo r;
                    r.log += "Failed to compile the shader to GLSL: \n";
                    r.log += session.GetLastErrorString();
                    r.errc = -4;
                    return std::unexpected(r);
                }

                std::string glsl = glslResult;
                return glsl;
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
