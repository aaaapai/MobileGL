// MobileGL - MobileGL/MG_Backend/Init.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Backends.h"
#include "MG_Util/Types.h"
#include <Config.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/MGToStr/GLExtensionConverter.h>

namespace MobileGL {
    namespace MG_Config {
        UniquePtr<RendererInfo> RendererInfoPtr = nullptr;
    } // namespace MG_Config

    namespace MG_Backend {
        void LogBackendInfo() {
            if (MG_Config::RendererInfoPtr) {
                MGLOG_I("MobileGL Backend Info:");
                MGLOG_I("  Renderer Name: %s", MG_Config::RendererInfoPtr->RendererName.c_str());
                MGLOG_I("  Backend Name: %s", MG_Config::RendererInfoPtr->BackendName.c_str());
                if (MG_Config::RendererInfoPtr->ExtraVendor) {
                    MGLOG_I("  Extra Vendor Info: %s", MG_Config::RendererInfoPtr->ExtraVendor->c_str());
                }
                MGLOG_I("  Target OpenGL Version: %d.%d",
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.Major,
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLVersion.Minor);
                MGLOG_I("  Target GLSL Version: %d.%d",
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLSLVersion.Major,
                        MG_Config::RendererInfoPtr->RendererGLInfo.TargetGLSLVersion.Minor);
                MGLOG_I("  OpenGL Extensions:");
                for (const auto& ext : MG_Config::RendererInfoPtr->RendererGLInfo.Extensions) {
                    MGLOG_I("    - %s", MG_Util::ConvertGLExtToString(ext).c_str());
                }
            } else {
                MGLOG_W("  No renderer info available");
            }
        }

        Bool InitSpecificBackendLibs() {
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_DILIGENT
            // Nothing to do
            MGLOG_D("Diligent Engine backend loaded");
            return true;
#elif MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            Bool result = MG_Util::BackendLoader::GLES::Init();
            MGLOG_D("DirectGLES backend loaded, GLES version: %d.%d", MG_External::GLES::g_glesCaps.version.Major,
                    MG_External::GLES::g_glesCaps.version.Minor);
            return result;
#else
            MGLOG_W("Unknown backend, skipping backend initialization");
            return false;
#endif
        }

        void Init() {
            MGLOG_D("Initializing MobileGL Backend...");

#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_DILIGENT
            switch (MG_Config::Backend::Diligent::SpecificBackend) {
            case MG_Backend::Diligent::SpecificBackendType::Vulkan:
                MG_Config::RendererInfoPtr = MakeUnique<RendererInfo>(Diligent::RendererInfoVulkan);
                break;
            case MG_Backend::Diligent::SpecificBackendType::Metal:
                MG_Config::RendererInfoPtr = MakeUnique<RendererInfo>(Diligent::RendererInfoMetal);
                break;
            default:
                throw RuntimeError("Unsupported renderer type");
            }
#elif MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Config::RendererInfoPtr = MakeUnique<RendererInfo>(DirectGLES::RendererInfo);
#else
            MG_Config::RendererInfoPtr = MakeUnique<RendererInfo>(Unknown::RendererInfoUnknown);
#endif

            Bool result = InitSpecificBackendLibs();
            if (!result) {
                MGLOG_W("Failed to initialize MobileGL backend libraries");
                return;
            }
            LogBackendInfo();
        }
    } // namespace MG_Backend
} // namespace MobileGL