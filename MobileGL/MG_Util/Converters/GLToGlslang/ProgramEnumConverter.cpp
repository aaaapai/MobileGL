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