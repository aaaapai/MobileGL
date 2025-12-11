#include "DirectGLES.h"
#include "Utils.h"
#include "Managers.h"
#include "MG_Util/Converters/GLToMG/FramebufferEnumConverter.h"
#include <MG_State/GLState/Core.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/FramebufferEnumConverter.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace BufferImpl {
        BackendBufferBindingProtector::BackendBufferBindingProtector(GLenum target) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            m_target = target;
            MG_External::GLES::glGetIntegerv(Utils::GetBindingQuery(target, false), &m_previousBinding);
        }

        BackendBufferBindingProtector::~BackendBufferBindingProtector() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            MG_External::GLES::glBindBuffer(m_target, m_previousBinding);
        }
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        BackendVertexArrayBindingProtector::BackendVertexArrayBindingProtector() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            MG_External::GLES::glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_previousBinding);
        }

        BackendVertexArrayBindingProtector::~BackendVertexArrayBindingProtector() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            MG_External::GLES::glBindVertexArray(m_previousBinding);
        }
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        BackendTextureBindingProtector::BackendTextureBindingProtector(GLenum target) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            m_target = target;
            MG_External::GLES::glGetIntegerv(Utils::GetBindingQuery(target, true), &m_previousBinding);
        }

        BackendTextureBindingProtector::~BackendTextureBindingProtector() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            MG_External::GLES::glBindTexture(m_target, m_previousBinding);
        }

        void NormalizePixelFormat(GLenum internalFormat, GLenum* outInternalFormat, GLenum* outType,
                                  GLenum* outFormat) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            switch (internalFormat) {
            case GL_DEPTH_COMPONENT16:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT24:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT32:
                if (outInternalFormat) *outInternalFormat = GL_DEPTH_COMPONENT32F;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT32F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;

            case GL_DEPTH_COMPONENT:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) *outFormat = GL_DEPTH_COMPONENT;
                break;
            case GL_DEPTH32F_STENCIL8:
            case GL_DEPTH_STENCIL:
                if (outInternalFormat) *outInternalFormat = GL_DEPTH32F_STENCIL8;
                if (outType) *outType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
                if (outFormat) *outFormat = GL_DEPTH_STENCIL;
                break;

            case GL_RGB10_A2:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_2_10_10_10_REV;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGB5_A1:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT_5_5_5_1;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_COMPRESSED_RED_RGTC1:
            case GL_COMPRESSED_RG_RGTC2:
                break;

            case GL_SRGB8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA32F:
            case GL_RGB32F:
            case GL_RG32F:
            case GL_R32F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) switch (internalFormat) {
                    case GL_RGBA32F:
                        if (outFormat) *outFormat = GL_RGBA;
                        break;
                    case GL_RGB32F:
                        if (outFormat) *outFormat = GL_RGB;
                        break;
                    case GL_RG32F:
                        if (outFormat) *outFormat = GL_RG;
                        break;
                    case GL_R32F:
                        if (outFormat) *outFormat = GL_RED;
                        break;
                    }
                break;

            case GL_RGB9_E5:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_5_9_9_9_REV;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_R11F_G11F_B10F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT_10F_11F_11F_REV;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA32UI:
            case GL_RGB32UI:
            case GL_RG32UI:
            case GL_R32UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_INT;
                if (outFormat) switch (internalFormat) {
                    case GL_RGBA32UI:
                        if (outFormat) *outFormat = GL_RGBA_INTEGER;
                        break;
                    case GL_RGB32UI:
                        if (outFormat) *outFormat = GL_RGB_INTEGER;
                        break;
                    case GL_RG32UI:
                        if (outFormat) *outFormat = GL_RG_INTEGER;
                        break;
                    case GL_R32UI:
                        if (outFormat) *outFormat = GL_RED_INTEGER;
                        break;
                    }
                break;

            case GL_RGBA32I:
            case GL_RGB32I:
            case GL_RG32I:
            case GL_R32I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_INT;
                if (outFormat) switch (internalFormat) {
                    case GL_RGBA32I:
                        if (outFormat) *outFormat = GL_RGBA_INTEGER;
                        break;
                    case GL_RGB32I:
                        if (outFormat) *outFormat = GL_RGB_INTEGER;
                        break;
                    case GL_RG32I:
                        if (outFormat) *outFormat = GL_RG_INTEGER;
                        break;
                    case GL_R32I:
                        if (outFormat) *outFormat = GL_RED_INTEGER;
                        break;
                    }
                break;

            case GL_RGBA16: {
                // TODO: check for extension GL_EXT_texture_norm16 for eligibility of (GL_RGBA16, GL_UNSIGNED_SHORT)
                // Most Mali does not support this (< Mali-G6xx, some G720?)
                if (outInternalFormat) *outInternalFormat = GL_RGBA16F;
                if (outType) *outType = GL_FLOAT;
                if (outFormat) *outFormat = GL_RGBA;
                break;
            }
            case GL_RGBA8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGBA:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            case GL_RGBA16F:
            case GL_R16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) {
                    if (internalFormat == GL_RGBA16F) {
                        *outFormat = GL_RGBA;
                    } else {
                        *outFormat = GL_RED;
                    }
                }
                break;

            case GL_RGB16:
                if (outInternalFormat) *outInternalFormat = GL_RGB16F;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGB16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RG16:
                if (outInternalFormat) *outInternalFormat = GL_RG16F;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_R8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_R8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_RED:
                // For GL_RED, we need to infer based on type or use default
                if (outInternalFormat) *outInternalFormat = GL_R8; // Default fallback
                if (outType) *outType = GL_UNSIGNED_BYTE;          // Default fallback
                if (outFormat) *outFormat = GL_RED;
                break;

            case GL_R8UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R8I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R16UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_R16I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RED_INTEGER;
                break;

            case GL_RG8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG16F:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_HALF_FLOAT;
                if (outFormat) *outFormat = GL_RG;
                break;

            case GL_RG8UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG8I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG16UI:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_SHORT;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;

            case GL_RG16I:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RG_INTEGER;
                break;


            case GL_RGB8_SNORM:
            case GL_RGBA8_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_BYTE;
                if (outFormat) {
                    if (internalFormat == GL_RGB8_SNORM) {
                        *outFormat = GL_RGB;
                    } else {
                        *outFormat = GL_RGBA;
                    }
                }
                break;

            case GL_RGB8:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE;
                if (outFormat) *outFormat = GL_RGB;
                break;

            case GL_RGBA16_SNORM:
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_SHORT;
                if (outFormat) *outFormat = GL_RGBA;
                break;

            default:
                // Fallback handling for other formats
                if (outInternalFormat) *outInternalFormat = internalFormat;
                if (outType) *outType = GL_UNSIGNED_BYTE; // More reasonable default
                if (outFormat) {
                    // Try to infer format from internal format name
                    if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RGBA") != nullptr) {
                        *outFormat = GL_RGBA;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RGB") != nullptr) {
                        *outFormat = GL_RGB;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RG") != nullptr) {
                        *outFormat = GL_RG;
                    } else if (strstr(MG_Util::ConvertGLEnumToString(internalFormat).c_str(), "RED") != nullptr) {
                        *outFormat = GL_RED;
                    } else {
                        *outFormat = GL_RGBA; // Ultimate fallback
                    }
                }
                break;
            }
        }

        void GenerateTextureFormatInfo(TextureInternalFormat internalFormat, GLenum* outInternalFormat, GLenum* outType,
                                       GLenum* outFormat) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            NormalizePixelFormat(MG_Util::ConvertTextureInternalFormatToGLEnum(internalFormat), outInternalFormat,
                                 outType, outFormat);
        }
    } // namespace TextureImpl

    namespace FramebufferImpl {
        BackendFramebufferBindingProtector::BackendFramebufferBindingProtector(GLenum target) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            m_target = target;
            MG_External::GLES::glGetIntegerv(Utils::GetBindingQuery(target, false), &m_previousBinding);
        }

        BackendFramebufferBindingProtector::~BackendFramebufferBindingProtector() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            MG_External::GLES::glBindFramebuffer(m_target, m_previousBinding);
        }

        GLuint BackendFramebufferBindingProtector::GetTempFBO(FramebufferTarget target) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            GLenum glTarget = MG_Util::ConvertFramebufferTargetToGLEnum(target);
            GLuint& fbo = (glTarget == GL_DRAW_FRAMEBUFFER) ? s_tempDrawFBO : s_tempReadFBO;
            if (fbo == 0) {
                MG_External::GLES::glGenFramebuffers(1, &fbo);
            }
            return fbo;
        }

        void BackendFramebufferBindingProtector::BindTempFBO(MobileGL::FramebufferTarget target) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            GLuint fbo = GetTempFBO(target);
            GLenum glTarget = MG_Util::ConvertFramebufferTargetToGLEnum(target);
            MG_External::GLES::glBindFramebuffer(glTarget, fbo);
        }
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        String ProcessOutColorLocations(const String& glslCode) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            const static std::regex pattern(R"(\n(out highp vec4 outColor)(\d+);)");
            const String replacement = "\nlayout(location=$2) $1$2;";
            return std::regex_replace(glslCode, pattern, replacement);
        }

        String ForceSupporterOutput(const String& glslCode) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            Bool hasPrecisionFloat =
                glslCode.find("precision ") != String::npos && glslCode.find("float;") != String::npos;
            Bool hasPrecisionInt = glslCode.find("precision ") != String::npos && glslCode.find("int;") != String::npos;

            String result = glslCode;
            String precisionFloat;
            String precisionInt;

            if (hasPrecisionFloat && hasPrecisionInt) {
                std::istringstream iss(result);
                std::vector<String> lines;
                String line;
                while (std::getline(iss, line)) {
                    Bool isPrecisionLine = (line.find("precision ") != String::npos) &&
                                           (line.find("float;") != String::npos || line.find("int;") != String::npos);
                    if (!isPrecisionLine) {
                        lines.push_back(line);
                    }
                }
                result.clear();
                for (SizeT i = 0; i < lines.size(); ++i) {
                    if (i != 0) result += '\n';
                    result += lines[i];
                }
                precisionFloat = "precision highp float;\n";
                precisionInt = "precision highp int;\n";
            } else {
                precisionFloat = hasPrecisionFloat ? "" : "precision highp float;\n";
                precisionInt = hasPrecisionInt ? "" : "precision highp int;\n";
            }

            SizeT lastExtensionPos = result.rfind("#extension");
            SizeT insertionPos = 0;

            if (lastExtensionPos != String::npos) {
                SizeT nextNewline = result.find('\n', lastExtensionPos);
                if (nextNewline != String::npos) {
                    insertionPos = nextNewline + 1;
                } else {
                    insertionPos = result.length();
                }
            } else {
                SizeT firstNewline = result.find('\n');
                if (firstNewline != String::npos) {
                    insertionPos = firstNewline + 1;
                } else {
                    result = precisionFloat + precisionInt + result;
                    return result;
                }
            }

            result.insert(insertionPos, precisionFloat + precisionInt);
            return result;
        }

        String RemoveLayoutBinding(const String& glslCode) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            static std::regex bindingRegex(R"(layout\s*\(\s*binding\s*=\s*\d+\s*\)\s*)");
            String result = std::regex_replace(glslCode, bindingRegex, "");
            static std::regex bindingRegex2(R"(layout\s*\(\s*binding\s*=\s*\d+\s*,)");
            result = std::regex_replace(result, bindingRegex2, "layout(");
            return result;
        }
    } // namespace PrgramImpl

    namespace Utils {
        void CheckGLESError() {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            while (GLenum err = MG_External::GLES::glGetError() != GL_NO_ERROR) {
                MGLOG_E("-> GLES Error: %s", MG_Util::ConvertGLEnumToString(err).c_str());
            }
        }

        GLenum GetBindingQuery(GLenum target, bool isTexture) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
            switch (target) {
            case GL_TEXTURE_BUFFER:
                return isTexture ? GL_TEXTURE_BINDING_BUFFER : GL_TEXTURE_BUFFER_BINDING;

            case GL_ARRAY_BUFFER:
                return GL_ARRAY_BUFFER_BINDING;
            case GL_ATOMIC_COUNTER_BUFFER:
                return GL_ATOMIC_COUNTER_BUFFER_BINDING;
            case GL_COPY_READ_BUFFER:
                return GL_COPY_READ_BUFFER_BINDING;
            case GL_COPY_WRITE_BUFFER:
                return GL_COPY_WRITE_BUFFER_BINDING;
            case GL_DISPATCH_INDIRECT_BUFFER:
                return GL_DISPATCH_INDIRECT_BUFFER_BINDING;
            case GL_DRAW_INDIRECT_BUFFER:
                return GL_DRAW_INDIRECT_BUFFER_BINDING;
            case GL_ELEMENT_ARRAY_BUFFER:
                return GL_ELEMENT_ARRAY_BUFFER_BINDING;
            case GL_PIXEL_PACK_BUFFER:
                return GL_PIXEL_PACK_BUFFER_BINDING;
            case GL_PIXEL_UNPACK_BUFFER:
                return GL_PIXEL_UNPACK_BUFFER_BINDING;
            case GL_QUERY_BUFFER:
                return GL_QUERY_BUFFER_BINDING;
            case GL_SHADER_STORAGE_BUFFER:
                return GL_SHADER_STORAGE_BUFFER_BINDING;
            case GL_TRANSFORM_FEEDBACK_BUFFER:
                return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
            case GL_UNIFORM_BUFFER:
                return GL_UNIFORM_BUFFER_BINDING;

            case GL_FRAMEBUFFER:
                return GL_FRAMEBUFFER_BINDING;
            case GL_DRAW_FRAMEBUFFER:
                return GL_DRAW_FRAMEBUFFER_BINDING;
            case GL_READ_FRAMEBUFFER:
                return GL_READ_FRAMEBUFFER_BINDING;

            case GL_RENDERBUFFER:
                return GL_RENDERBUFFER_BINDING;

            case GL_VERTEX_ARRAY:
                return GL_VERTEX_ARRAY_BINDING;
            case GL_VERTEX_ARRAY_BINDING:
                return GL_VERTEX_ARRAY_BINDING;

            case GL_PROGRAM_PIPELINE:
                return GL_PROGRAM_PIPELINE_BINDING;

            case GL_PROGRAM:
                return GL_CURRENT_PROGRAM;

            case GL_SAMPLER:
                return GL_SAMPLER_BINDING;

            case GL_TEXTURE:
                return GL_TEXTURE_BINDING_2D;
            case GL_TEXTURE_1D:
                return GL_TEXTURE_BINDING_1D;
            case GL_TEXTURE_1D_ARRAY:
                return GL_TEXTURE_BINDING_1D_ARRAY;
            case GL_TEXTURE_2D:
                return GL_TEXTURE_BINDING_2D;
            case GL_TEXTURE_2D_ARRAY:
                return GL_TEXTURE_BINDING_2D_ARRAY;
            case GL_TEXTURE_2D_MULTISAMPLE:
                return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
            case GL_TEXTURE_3D:
                return GL_TEXTURE_BINDING_3D;
            case GL_TEXTURE_CUBE_MAP:
                return GL_TEXTURE_BINDING_CUBE_MAP;
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
            case GL_TEXTURE_RECTANGLE:
                return GL_TEXTURE_BINDING_RECTANGLE;

            case GL_TRANSFORM_FEEDBACK:
                return GL_TRANSFORM_FEEDBACK_BINDING;

            case GL_SAMPLES_PASSED:
                return GL_SAMPLES_PASSED;
            case GL_PRIMITIVES_GENERATED:
                return GL_PRIMITIVES_GENERATED;

            case GL_DEBUG_OUTPUT:
                return GL_DEBUG_OUTPUT;
            case GL_DEBUG_OUTPUT_SYNCHRONOUS:
                return GL_DEBUG_OUTPUT_SYNCHRONOUS;

            default:
                return 0;
            }
        }
    } // namespace Utils
} // namespace MobileGL::MG_Backend::DirectGLES
