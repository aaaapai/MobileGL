// MobileGL - MobileGL/MG_Backend/Backends.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/GLExtensions.h>

namespace MobileGL {
    namespace MG_Backend {
        namespace Unknown {
            inline RendererInfo RendererInfoUnknown = {
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
                .BackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
            };
        }

        namespace Diligent {
            enum class SpecificBackendType {
                Vulkan,
                Metal
            };

            inline RendererInfo RendererInfoVulkan = {
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
                .BackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
            };

            inline RendererInfo RendererInfoMetal = {
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
                .BackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
            };
        } // namespace Diligent

        namespace DirectGLES {
            inline RendererInfo RendererInfo = {
                .RendererName = "Espryt",            // Renderer Name
                .BackendName = "Direct (OpenGL ES)", // Backend Name
                .ExtraVendor = Nullopt,              // Extra vendor
                .RendererGLInfo =
                    {
                        // OpenGL Info
                        .TargetGLVersion = {4, 3, 0},                      //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0},                    //   Target Shading Language Version
                        .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                                       V_OpenGL33/*, V_OpenGL40, V_OpenGL41, V_OpenGL42, V_OpenGL43,*/ E_GL_ARB_draw_buffers_blend},
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
            };
        } // namespace DirectGLES

        void Init();
    } // namespace MG_Backend
} // namespace MobileGL
