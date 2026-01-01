// MobileGL - MobileGL/MG_Util/Converters/GLToGlslang/ProgramEnumConverter.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ShadercEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertGLEnumToShadercGlsl(GLenum shaderType) {
            switch (shaderType) {
            case GL_VERTEX_SHADER:
                return shaderc_glsl_vertex_shader;
            case GL_FRAGMENT_SHADER:
                return shaderc_glsl_fragment_shader;
            case GL_COMPUTE_SHADER:
                return shaderc_glsl_compute_shader;
            case GL_TESS_CONTROL_SHADER:
                return shaderc_glsl_tess_control_shader;
            case GL_TESS_EVALUATION_SHADER:
                return shaderc_glsl_tess_evaluation_shader;
            case GL_GEOMETRY_SHADER:
                return shaderc_glsl_geometry_shader;
            default:
                return shaderc_glsl_infer_from_source;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL
