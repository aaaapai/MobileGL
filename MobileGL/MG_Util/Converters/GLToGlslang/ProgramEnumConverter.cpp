// MobileGL - MobileGL/MG_Util/Converters/GLToGlslang/ProgramEnumConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ProgramEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        EShLanguage ConvertGLEnumToEShLanguage(GLenum shaderType) {
            switch (shaderType) {
            case GL_VERTEX_SHADER:
                return EShLanguage::EShLangVertex;
            case GL_FRAGMENT_SHADER:
                return EShLanguage::EShLangFragment;
            case GL_COMPUTE_SHADER:
                return EShLanguage::EShLangCompute;
            case GL_TESS_CONTROL_SHADER:
                return EShLanguage::EShLangTessControl;
            case GL_TESS_EVALUATION_SHADER:
                return EShLanguage::EShLangTessEvaluation;
            case GL_GEOMETRY_SHADER:
                return EShLanguage::EShLangGeometry;
            default:
                return EShLanguage::EShLangCount;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL