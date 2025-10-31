#include "ProgramEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertShaderStageToGLEnum(ShaderStage stage) {
            switch (stage) {
            case ShaderStage::Vertex:
                return GL_VERTEX_SHADER;
            case ShaderStage::TessControl:
                return GL_TESS_CONTROL_SHADER;
            case ShaderStage::TessEval:
                return GL_TESS_EVALUATION_SHADER;
            case ShaderStage::Geometry:
                return GL_GEOMETRY_SHADER;
            case ShaderStage::Fragment:
                return GL_FRAGMENT_SHADER;
            case ShaderStage::Compute:
                return GL_COMPUTE_SHADER;
            default:
                return GL_UNKNOWN_MGL;
            }
        }
    } // namespace MG_Util
} // namespace MobileGL