#pragma once
#include <Includes.h>
#include <MG_Backend/Backends.h>

namespace MobileGL {
    namespace MG_Config {
        inline const String ProjectName = "MobileGL";
        inline const String CoreName = "MobileGL Core";
        inline const String CoreVendor = "MobileGL-Dev";
        inline const Version CoreVersion = {1, 0, 0, "-Dev"};
        extern RendererInfo* RendererInfoPtr;

        namespace Backend {
#define MOBILEGL_BACKEND MOBILEGL_BACKEND_UNKNOWN

            namespace Diligent {
                inline const MG_Backend::Diligent::SpecificBackendType SpecificBackend =
                    MG_Backend::Diligent::SpecificBackendType::Vulkan;
            }
        } // namespace Backend
    } // namespace MG_Config
} // namespace MobileGL