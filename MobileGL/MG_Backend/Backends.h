#pragma once

#include <Includes.h>
#include <MG_Util/GLExtensions.h>

namespace MobileGL {
    namespace MG_Backend {
        namespace Unknown {
            const RendererInfo RendererInfoUnknown = {
                .RendererName = "<unknown renderer of MobileGL>", // Renderer Name
                .BackendName = "<unknown backend>",               // Backend Name
                .ExtraVendor = ", <unknown>",                     // Extra vendor
                .RendererGLInfo =
                    {
                        // OpenGL Info
                        .TargetGLVersion = {3, 3, 0},   //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0}, //   Target Shading Language Version
                        .Extensions = {},               //   OpenGL Extensions
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {} // Backend Capability
            };
        }

        namespace Diligent {
            enum class SpecificBackendType {
                Vulkan,
                Metal
            };

            const RendererInfo RendererInfoVulkan = {
                .RendererName = "MG-DE-Vulkan",            // Renderer Name
                .BackendName = "Diligent Engine (Vulkan)", // Backend Name
                .ExtraVendor = Nullopt,                    // Extra vendor
                .RendererGLInfo =
                    {
                        // OpenGL Info
                        .TargetGLVersion = {3, 3, 0},                      //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0},                    //   Target Shading Language Version
                        .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                                       V_OpenGL33},
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {} // Backend Capability
            };

            const RendererInfo RendererInfoMetal = {
                .RendererName = "MG-DE-Metal",            // Renderer Name
                .BackendName = "Diligent Engine (Metal)", // Backend Name
                .ExtraVendor = Nullopt,                   // Extra vendor
                .RendererGLInfo =
                    {
                        // OpenGL Info
                        .TargetGLVersion = {3, 3, 0},                      //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0},                    //   Target Shading Language Version
                        .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                                       V_OpenGL33},
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {} // Backend Capability
            };
        } // namespace Diligent

        namespace DirectGLES {
            const RendererInfo RendererInfo = {
                .RendererName = "Espryt",            // Renderer Name
                .BackendName = "Direct (OpenGL ES)", // Backend Name
                .ExtraVendor = Nullopt,              // Extra vendor
                .RendererGLInfo =
                    {
                        // OpenGL Info
                        .TargetGLVersion = {3, 3, 0},                      //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0},                    //   Target Shading Language Version
                        .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                                       V_OpenGL33},
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {} // Backend Capability
            };
        } // namespace DirectGLES

        void Init();
    } // namespace MG_Backend
} // namespace MobileGL
