// MobileGL - MobileGL/MG_Util/Converters/GLToMG/ProgramEnumConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ProgramEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        ShaderStage ConvertGLEnumToShaderStage(GLenum type) {
            switch (type) {
            case GL_VERTEX_SHADER:
                return ShaderStage::Vertex;
            case GL_TESS_CONTROL_SHADER:
                return ShaderStage::TessControl;
            case GL_TESS_EVALUATION_SHADER:
                return ShaderStage::TessEval;
            case GL_GEOMETRY_SHADER:
                return ShaderStage::Geometry;
            case GL_FRAGMENT_SHADER:
                return ShaderStage::Fragment;
            case GL_COMPUTE_SHADER:
                return ShaderStage::Compute;
            default:
                return ShaderStage::Unknown;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL