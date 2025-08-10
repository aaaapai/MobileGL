#include "GL_Getter.h"
#include <Config.h>
#include <MG_State/GLState/Core.h>
#include <MG_Util/Converters/MGToGL/ErrorCodeConverter.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToStr/GLExtensionConverter.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        const GLubyte* GetString(GLenum name) {
            static String vendorString;
            static String versionStr;
            static String rendererString;
            static String shadingLanguageVersion;
            static String extensionsString;
            MGLOG_D("glGetString, name: %s", MG_Util::ConvertGLEnumToString(name).c_str());
            switch (name) {
            case GL_VENDOR:
                if (vendorString.empty()) {
                    if (MG_Config::RendererInfoPtr->ExtraVendor.has_value()) {
                        vendorString =
                            std::format("{}{}", MG_Config::CoreVendor, MG_Config::RendererInfoPtr->ExtraVendor.value());
                    } else {
                        vendorString = MG_Config::CoreVendor;
                    }
                }
                return (const GLubyte*)vendorString.c_str();
            case GL_VERSION: {
                if (versionStr.empty()) {
                    versionStr =
                        std::format("{} {}, {} Backend, Version {}",
                                    MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.toString().c_str(),
                                    MG_Config::ProjectName.c_str(), MG_Config::RendererInfoPtr->BackendName.c_str(),
                                    MG_Config::CoreVersion.toString().c_str());
                }
                return (const GLubyte*)versionStr.c_str();
            }

            case GL_RENDERER: {
                if (rendererString.empty()) {
                    String deviceInfo;
                    rendererString = std::format("{} ({}) ({})", MG_Config::RendererInfoPtr->RendererName.c_str(),
                                                 MG_Config::CoreName.c_str(), deviceInfo);
                }
                return (const GLubyte*)rendererString.c_str();
            }
            case GL_SHADING_LANGUAGE_VERSION:
                if (shadingLanguageVersion.empty()) {
                    shadingLanguageVersion = std::format(
                        "{} MobileGL",
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLSLVersion.toString({true, false}).c_str());
                }
                return (const GLubyte*)shadingLanguageVersion.c_str();
            case GL_EXTENSIONS:
                if (extensionsString.empty()) {
                    for (auto& ext : MG_Config::RendererInfoPtr->RendererGLInfo.Extensions) {
                        extensionsString += MG_Util::ConvertetGLExtToString(ext);
                        extensionsString += " ";
                    }
                    extensionsString.pop_back();
                }
                return (const GLubyte*)extensionsString.c_str();
            default:
                return (const GLubyte*)"Unknown enum";
            }
        }

        const GLubyte* GetStringi(GLenum name, GLuint index) {
            MGLOG_D("glGetStringi, name: %s, index: %u", MG_Util::ConvertGLEnumToString(name).c_str(), index);
            if (name != GL_EXTENSIONS) {
                return (const GLubyte*)"";
            }

            const auto& exts = MG_Config::RendererInfoPtr->RendererGLInfo.Extensions;
            if (index >= exts.size()) {
                return nullptr;
            }

            static Vector<String> extStrings;
            static Bool initialized = false;
            if (!initialized) {
                extStrings.reserve(exts.size());
                for (const auto& ext : exts) {
                    extStrings.emplace_back(MG_Util::ConvertetGLExtToString(ext));
                }
                initialized = true;
            }

            return (const GLubyte*)extStrings[index].c_str();
        }

        void GetIntegerv(GLenum pname, GLint* params) {
            MGLOG_D("glGetIntegerv, pname: %s", MG_Util::ConvertGLEnumToString(pname).c_str());
            if (!params) {
                // TODO: report GL_INVALID_VALUE.
                return;
            }

            switch (pname) {
                // General
            case GL_CONTEXT_FLAGS:
                params[0] = 0;
            case GL_CONTEXT_PROFILE_MASK:
                params[0] = MG_Config::RendererInfoPtr->RendererGLInfo.IsCompatibilityProfile
                                ? GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
                                : GL_CONTEXT_CORE_PROFILE_BIT;
                break;
            case GL_NUM_EXTENSIONS:
                params[0] = MG_Config::RendererInfoPtr->RendererGLInfo.Extensions.size();
                break;
            case GL_MAJOR_VERSION:
                params[0] = MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.Major;
                break;
            case GL_MINOR_VERSION:
                params[0] = MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.Minor;
                break;

                // State
                // TODO

                // Others...
            default:
                MGLOG_E("glGetIntegerv: Invalid enum %s (0x%X)", MG_Util::ConvertGLEnumToString(pname).c_str(), pname);
                // TODO: report GL_INVALID_ENUM.
                break;
            }
        }

        GLenum GetError() {
            ErrorCode errorCode = MG_State::pGLContext->PopGLError().value_or(Error{ErrorCode::NoError, nullptr}).code;
            return MG_Util::ConvertErrorCodeToGLEnum(errorCode);
        }
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL