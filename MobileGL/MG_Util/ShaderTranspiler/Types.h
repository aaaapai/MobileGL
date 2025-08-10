#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            inline const char* GLOBAL_UBO_NAME = "MGL_GLOBAL_UBO";

            struct EmptyType {};

            enum class ShaderCompileBits : Uint {
                EmitDiscardAsDemote = 1 << 0,
            };

            struct ShaderAttrib {
                GLenum shaderType;
                StringView sourceStr;
                Flags<ShaderCompileBits> flags;
            };

            struct ProgramAttrib {
                Vector<GLenum> shaderTypes;
                Vector<SharedPtr<glslang::TShader>> shaders;
            };

            struct ResultInfo {
                Int errc = 0;
                String log;
            };

            template <typename T>
            using Result = std::expected<T, ResultInfo>;

            struct InterfaceVariable {
                std::string name;
                uint32_t location;

                Bool operator<(const InterfaceVariable& other) const { return location < other.location; }

                Bool operator==(const InterfaceVariable& other) const {
                    return location == other.location && name == other.name;
                }
            };

            // class SpvcSession {
            // public:
            //     SpvcSession() {}
            //
            //     explicit SpvcSession(const Vector<unsigned int>& spirv) {
            //         const SpvId *p_spirv = spirv.data();
            //         size_t word_count = spirv.size();
            //
            //         spvc_context_create(&context);
            //         spvc_context_parse_spirv(context, p_spirv, word_count, &ir);
            //         spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
            //         &compiler); spvc_compiler_create_shader_resources(compiler, &resources);
            //     }
            //
            //     SpvcSession(SpvcSession&) = delete;
            //
            //     SpvcSession(SpvcSession&& that) {
            //         std::swap(this->context, that.context);
            //         std::swap(this->compiler, that.compiler);
            //         std::swap(this->ir, that.ir);
            //         std::swap(this->compiler_options, that.compiler_options);
            //         std::swap(this->resources, that.resources);
            //     }
            //
            //     SpvcSession& operator=(SpvcSession& session) = delete;
            //
            //     SpvcSession& operator=(SpvcSession&& that) {
            //         std::swap(this->context, that.context);
            //         std::swap(this->compiler, that.compiler);
            //         std::swap(this->ir, that.ir);
            //         std::swap(this->compiler_options, that.compiler_options);
            //         std::swap(this->resources, that.resources);
            //         return *this;
            //     }
            //
            //     spvc_result CreateOptions(spvc_compiler_options *options) {
            //         return spvc_compiler_create_compiler_options(compiler, options);
            //     }
            //
            //     spvc_result SetOptions(spvc_compiler_options options) {
            //         compiler_options = options;
            //         return spvc_compiler_install_compiler_options(compiler, options);
            //     }
            //
            //     Vector<InterfaceVariable> GetShaderInterface(spvc_resource_type resource_type) const {
            //         const spvc_reflected_resource *list = nullptr;
            //         size_t count = 0;
            //         spvc_resources_get_resource_list_for_type(resources, resource_type, &list, &count);
            //
            //         Vector<InterfaceVariable> variables;
            //         for (size_t i = 0; i < count; ++i) {
            //             if (spvc_compiler_has_decoration(compiler, list[i].id, SpvDecorationBuiltIn)) {
            //                 continue;
            //             }
            //
            //             InterfaceVariable var;
            //             var.name = list[i].name;
            //             var.location = spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationLocation);
            //             variables.push_back(var);
            //
            //             if (resource_type == SPVC_RESOURCE_TYPE_UNIFORM_BUFFER) {
            //                 spvc_type type = spvc_compiler_get_type_handle(compiler, list[i].base_type_id);
            //                 size_t num_members = spvc_type_get_num_member_types(type);
            //                 printf("uniform %s {\n", var.name.c_str());
            //                 for (size_t j = 0; j < num_members; ++j) {
            //                     const char *memberName = spvc_compiler_get_member_name(compiler,
            //                     list[i].base_type_id, j);
            //                     // auto memberTypeId = spvc_type_get_member_type(type, j);
            //                     // auto memberType = spvc_compiler_get_type_handle(compiler, memberTypeId);
            //
            //                     unsigned memberOffset = 0;
            //                     spvc_compiler_type_struct_member_offset(compiler, type, j, &memberOffset);
            //                     printf("\b%s; // %u\n", memberName, memberOffset);
            //                 }
            //                 printf("}\n");
            //             }
            //         }
            //         std::sort(variables.begin(), variables.end());
            //         return variables;
            //     }
            //
            //     spvc_result Compile(const char** result) const {
            //         return spvc_compiler_compile(compiler, result);
            //     }
            //
            //     const char* GetLastErrorString() const {
            //         return spvc_context_get_last_error_string(context);
            //     }
            //
            //     ~SpvcSession() { spvc_context_destroy(context); }
            // private:
            //     spvc_context context = nullptr;
            //     spvc_parsed_ir ir = nullptr;
            //     spvc_compiler compiler = nullptr;
            //     spvc_compiler_options compiler_options = nullptr;
            //     spvc_resources resources = nullptr;
            // };

            inline static EShLanguage GetEShLanguageByShaderType(GLenum shaderType) {
                switch (shaderType) {
                case GL_VERTEX_SHADER:
                    return EShLanguage::EShLangVertex;
                case GL_FRAGMENT_SHADER:
                    return EShLanguage::EShLangFragment;
                case GL_COMPUTE_SHADER:
                    return EShLanguage::EShLangCompute;
                case GL_TESS_CONTROL_SHADER:
                    return EShLanguage::EShLangTessControl;
                case GL_TESS_EVALUATION_SHADER:
                    return EShLanguage::EShLangTessEvaluation;
                case GL_GEOMETRY_SHADER:
                    return EShLanguage::EShLangGeometry;
                default:
                    return EShLanguage::EShLangCount;
                }
            }
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
