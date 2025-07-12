#include "../../../Includes.h"

namespace MobileGL {
	namespace MG_Impl::GLImpl {
        const GLubyte* GetString(GLenum name) {
            MGLOG_D("glGetString, name: %s", MG_Util::ConvertGLEnumToString(name).c_str());
            switch (name) {
            case GL_VENDOR:
                static String vendorString;
                if (vendorString.empty()) {
                    if (MG_Config::RendererInfoPtr->ExtraVendor.has_value()) {
                        vendorString = std::format("{}{}",
                            MG_Config::CoreVendor, MG_Config::RendererInfoPtr->ExtraVendor.value());
                    } else {
						vendorString = MG_Config::CoreVendor;
                    }
                }
                return (const GLubyte*)vendorString.c_str();
            case GL_VERSION: {
                static String versionStr;
                if (versionStr.empty()) {
                    versionStr = std::format("{} {}, {} Backend, Version {}",
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.toString().c_str(),
                        MG_Config::ProjectName.c_str(),
                        MG_Config::RendererInfoPtr->BackendName.c_str(),
                        MG_Config::CoreVersion.toString().c_str());
                }
                return (const GLubyte*)versionStr.c_str();
            }

            case GL_RENDERER:
            {
                static String rendererString;
                if (rendererString.empty()) {
                    String deviceInfo;
                    rendererString = std::format("{} ({}) ({})",
                        MG_Config::RendererInfoPtr->RendererName.c_str(),
                        MG_Config::CoreName.c_str(),
                        deviceInfo);
                }
                return (const GLubyte*)rendererString.c_str();
            }
            case GL_SHADING_LANGUAGE_VERSION:
                static String shadingLanguageVersion;
                if (shadingLanguageVersion.empty()) {
                    shadingLanguageVersion = std::format("{} MobileGL", 
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLSLVersion.toString().c_str());
                }
                return (const GLubyte*)shadingLanguageVersion.c_str();
            case GL_EXTENSIONS:
                static String extensionsString;
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
            static bool initialized = false;
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
                // TODO: Report GL_INVALID_VALUE.
                return;
            }

            switch (pname) {
                // General
            case GL_CONTEXT_PROFILE_MASK:
                params[0] = GL_CONTEXT_CORE_PROFILE_BIT;
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
                MGLOG_E("glGetIntegerv: Invalid enum %s (0x%X)",
                    MG_Util::ConvertGLEnumToString(pname).c_str(), pname);
				// TODO: Report GL_INVALID_ENUM.
                break;
            }
        }
	}
}