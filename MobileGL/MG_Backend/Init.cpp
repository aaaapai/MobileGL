// MobileGL - MobileGL/MG_Backend/Init.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "BackendObjects.h"
#include <Config.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/MGToStr/GLExtensionConverter.h>

namespace MobileGL::MG_Backend {
    void LogBackendInfo() {
        if (!pActiveBackendObject) {
            MGLOG_W("No active backend object, cannot log backend info");
            return;
        }

        const auto& rendererInfo = pActiveBackendObject->GetRendererInfo();
        MGLOG_I("MobileGL Backend Info:");
        MGLOG_I("  Renderer Name: %s", rendererInfo.RendererName.c_str());
        MGLOG_I("  Backend Name: %s", rendererInfo.BackendName.c_str());
        if (rendererInfo.ExtraVendor) {
            MGLOG_I("  Extra Vendor Info: %s", rendererInfo.ExtraVendor->c_str());
        }
        MGLOG_I("  Target OpenGL Version: %d.%d", rendererInfo.RendererGLInfo.TargetGLVersion.Major,
                rendererInfo.RendererGLInfo.TargetGLVersion.Minor);
        MGLOG_I("  Target GLSL Version: %d.%d", rendererInfo.RendererGLInfo.TargetGLSLVersion.Major,
                rendererInfo.RendererGLInfo.TargetGLSLVersion.Minor);
        MGLOG_I("  OpenGL Extensions:");
        for (const auto& ext : rendererInfo.RendererGLInfo.Extensions) {
            MGLOG_I("    - %s", MG_Util::ConvertGLExtToString(ext).c_str());
        }
    }

    Bool InitSpecificBackendLibs() {
        if (!pActiveBackendObject) {
            MGLOG_W("No active backend object, cannot initialize backend libraries");
            return false;
        }
        pActiveBackendObject->Initialize();
        gBackendFunctionsTable = pActiveBackendObject->GetBackendFunctions();
        return true;
    }

    void Init() {
        MGLOG_D("Initializing MobileGL Backend...");

        switch (MG_Config::ActiveBackendType) {
        case BackendType::DirectGLES: {

            const auto& directGLESInfo = pActiveBackendObject->GetRendererInfo();

            // 检查环境变量LIBGL_GL
            const char* envLibGL = std::getenv("LIBGL_GL");
            const char* envlibGL_compute = std::getenv("LIBGL_COMPUTE_SHADER");
            if ((envLibGL != nullptr) || (envlibGL_compute != nullptr)) {
                std::string libglValue = "";
                if (envLibGL != nullptr) {
                    libglValue = envLibGL;
                }
                auto& extensions = directGLESInfo.RendererGLInfo.Extensions;

                if (envlibGL_compute != nullptr) {
                    extensions.push_back(E_GL_ARB_compute_shader);
                }
        
                // 如果设置为"43"，则使用OpenGL 4.3
                if ((!libglValue.empty()) && (libglValue == "43")) {
                    MGLOG_I("LIBGL_GL=43 detected, using OpenGL 4.3 configuration");
                    
                    // 修改OpenGL版本
                    directGLESInfo.RendererGLInfo.TargetGLVersion = {4, 3, 0};
                    
                    // 添加OpenGL 4.x扩展
                    extensions.push_back(V_OpenGL40);
                    extensions.push_back(V_OpenGL41);
                    extensions.push_back(V_OpenGL42);
                    extensions.push_back(V_OpenGL43);
                    
                }

            }

            pActiveBackendObject = MakeUnique<DirectGLES::BackendObject_DirectGLES>();
            break;
        }
        case BackendType::Unknown:
        default:
            MGLOG_W("Unknown backend type, defaulting to unknown backend");
            pActiveBackendObject = nullptr;
        }

        Bool result = InitSpecificBackendLibs();
        if (!result) {
            MGLOG_W("Failed to initialize MobileGL backend libraries");
            return;
        }
        LogBackendInfo();
    }
} // namespace MobileGL::MG_Backend
