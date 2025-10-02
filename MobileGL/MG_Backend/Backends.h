#pragma once

#include <Includes.h>
#include <MG_Util/GLExtensions.h>

namespace MobileGL {
    namespace MG_Backend {
        namespace Unknown {
            const RendererInfo RendererInfoUnknown = {
                "<unknown renderer of MobileGL>", // Renderer Name
                "<unknown backend>",              // Backend Name
                ", <unknown>",                    // Extra vendor
                {
                    // OpenGL Info
                    {3, 3, 0}, //   Target OpenGL Version
                    {4, 6, 0}, //   Target Shading Language Version
                    {},        //   OpenGL Extensions
                    false      //   Is Compatibility Profile
                },
                {} // Backend Capability
            };
        }

        namespace Diligent {
            enum class SpecificBackendType {
                Vulkan,
                Metal
            };

            const RendererInfo RendererInfoVulkan = {
                "MobileGlued-vk",           // Renderer Name
                "Diligent Engine (Vulkan)", // Backend Name
                Nullopt,                    // Extra vendor
                {
                    // OpenGL Info
                    {3, 3, 0},                           //   Target OpenGL Version
                    {4, 6, 0},                           //   Target Shading Language Version
                    {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                     V_OpenGL33},
                    false //   Is Compatibility Profile
                },
                {} // Backend Capability
            };

            const RendererInfo RendererInfoMetal = {
                "MobileGlued-mtl",         // Renderer Name
                "Diligent Engine (Metal)", // Backend Name
                Nullopt,                   // Extra vendor
                {
                    // OpenGL Info
                    {3, 3, 0},                           //   Target OpenGL Version
                    {4, 6, 0},                           //   Target Shading Language Version
                    {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                     V_OpenGL33},
                    false //   Is Compatibility Profile
                },
                {} // Backend Capability
            };
        } // namespace Diligent

        void Init();
    } // namespace MG_Backend
} // namespace MobileGL
