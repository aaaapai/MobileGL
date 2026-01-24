// MobileGL - MobileGL/MG_Backend/Backends.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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
                        .TargetGLVersion = {3, 3, 0},                      //   Target OpenGL Version
                        .TargetGLSLVersion = {4, 6, 0},                    //   Target Shading Language Version
                        .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32, //   OpenGL Extensions
                                       V_OpenGL33, E_GL_ARB_shader_image_load_store, E_GL_ARB_draw_buffers_blend /*E_GL_ARB_shader_storage_buffer_object*/ ,E_GL_ARB_compute_shader, GL_EXT_timer_query, GL_ARB_timer_query},
                        .IsCompatibilityProfile = false //   Is Compatibility Profile
                    },
                .BackendCapability = {.AllowVSOnlyPrograms = false} // Backend Capability
            };
        } // namespace DirectGLES

        void Init();
    } // namespace MG_Backend
} // namespace MobileGL
