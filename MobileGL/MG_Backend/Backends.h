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
            inline RendererInfo RendererInfoObj = []() -> RendererInfo {
                // 默认值（对应LIBGL_GL=33的情况）
                RendererInfo info = {
                    .RendererName = "Espryt",
                    .BackendName = "Direct (OpenGL ES)",
                    .ExtraVendor = Nullopt,
                    .RendererGLInfo =
                        {
                            .TargetGLVersion = {3, 3, 0},
                            .TargetGLSLVersion = {4, 6, 0},
                            .Extensions = {V_OpenGL30, V_OpenGL31, V_OpenGL32,
                                           V_OpenGL33, E_GL_ARB_shader_image_load_store, 
                                           E_GL_ARB_draw_buffers_blend, E_GL_ARB_shader_storage_buffer_object},
                            .IsCompatibilityProfile = false
                        },
                    .BackendCapability = {.AllowVSOnlyPrograms = false}
                };
        
                // 检查环境变量LIBGL_GL
                const char* envLibGL = std::getenv("LIBGL_GL");
                if (envLibGL != nullptr) {
                    std::string libglValue = envLibGL;
            
                    // 如果设置为"43"，则使用OpenGL 4.3
                    if (libglValue == "43") {
                        // 修改OpenGL版本
                        info.RendererGLInfo.TargetGLVersion = {4, 3, 0};
                
                        // 添加OpenGL 4.x扩展
                        auto& extensions = info.RendererGLInfo.Extensions;
                        extensions.push_back(V_OpenGL40);
                        extensions.push_back(V_OpenGL41);
                        extensions.push_back(V_OpenGL42);
                        extensions.push_back(V_OpenGL43);
                    }
                }
        
                return info;
            }();
        } // namespace DirectGLES

        void Init();
    } // namespace MG_Backend
} // namespace MobileGL
