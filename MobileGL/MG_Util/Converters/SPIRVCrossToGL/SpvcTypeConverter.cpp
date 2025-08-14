//
// Created by Swung0x48 on 2025/8/14.
//

#include "SpvcTypeConverter.h"

#include "MG_Util/ShaderTranspiler/SpvcSession.h"

namespace MobileGL {
    namespace MG_Util {
        GLenum ConvertBaseTypeToType(GLenum baseType, Uint vecSize, Uint matCol) {
            if (vecSize < 1 || vecSize > 4 || matCol < 1 || matCol > 4) {
                return GL_FALSE;
            }

            if (vecSize == 1 && matCol == 1) {
                return baseType;
            }

            if (matCol > 1) {
                switch (baseType) {
                    case GL_FLOAT:
                        if (vecSize == 2 && matCol == 2) return GL_FLOAT_MAT2;
                        if (vecSize == 2 && matCol == 3) return GL_FLOAT_MAT3x2;
                        if (vecSize == 2 && matCol == 4) return GL_FLOAT_MAT4x2;
                        if (vecSize == 3 && matCol == 2) return GL_FLOAT_MAT2x3;
                        if (vecSize == 3 && matCol == 3) return GL_FLOAT_MAT3;
                        if (vecSize == 3 && matCol == 4) return GL_FLOAT_MAT4x3;
                        if (vecSize == 4 && matCol == 2) return GL_FLOAT_MAT2x4;
                        if (vecSize == 4 && matCol == 3) return GL_FLOAT_MAT3x4;
                        if (vecSize == 4 && matCol == 4) return GL_FLOAT_MAT4;
                        break;
                    case GL_DOUBLE:
                        if (vecSize == 2 && matCol == 2) return GL_DOUBLE_MAT2;
                        if (vecSize == 2 && matCol == 3) return GL_DOUBLE_MAT3x2;
                        if (vecSize == 2 && matCol == 4) return GL_DOUBLE_MAT4x2;
                        if (vecSize == 3 && matCol == 2) return GL_DOUBLE_MAT2x3;
                        if (vecSize == 3 && matCol == 3) return GL_DOUBLE_MAT3;
                        if (vecSize == 3 && matCol == 4) return GL_DOUBLE_MAT4x3;
                        if (vecSize == 4 && matCol == 2) return GL_DOUBLE_MAT2x4;
                        if (vecSize == 4 && matCol == 3) return GL_DOUBLE_MAT3x4;
                        if (vecSize == 4 && matCol == 4) return GL_DOUBLE_MAT4;
                        break;
                    default:
                        return GL_FALSE;
                }
            }

            else if (matCol == 1) {
                switch (baseType) {
                    case GL_BOOL:
                        switch (vecSize) {
                            case 2: return GL_BOOL_VEC2;
                            case 3: return GL_BOOL_VEC3;
                            case 4: return GL_BOOL_VEC4;
                        }
                    case GL_FLOAT:
                        return GL_FLOAT_VEC2 + (vecSize - 2); // GL_FLOAT_VEC* is contiguous
                    case GL_DOUBLE:
                         switch (vecSize) {
                            case 2: return GL_DOUBLE_VEC2;
                            case 3: return GL_DOUBLE_VEC3;
                            case 4: return GL_DOUBLE_VEC4;
                         }
                         break;
                    case GL_INT:
                        return GL_INT_VEC2 + (vecSize - 2); // GL_INT_VEC* is contiguous
                    case GL_UNSIGNED_INT:
                        return GL_UNSIGNED_INT_VEC2 + (vecSize - 2); // GL_UNSIGNED_INT_VEC* is contiguous
                }
            }

            return GL_FALSE;
        }


        GLenum ConvertSpvcTypeToGLEnum(const ShaderTranspiler::SpvcType& spvcType) {
            switch (spvcType.basetype) {
                case SPVC_BASETYPE_BOOLEAN:
                    return ConvertBaseTypeToType(GL_BOOL, spvcType.vectorSize, spvcType.matCol);
                case SPVC_BASETYPE_INT8:
                    return ConvertBaseTypeToType(GL_BYTE, spvcType.vectorSize, spvcType.matCol);

                case SPVC_BASETYPE_INT16:
                case SPVC_BASETYPE_INT32:
                case SPVC_BASETYPE_INT64:
                    return ConvertBaseTypeToType(GL_INT, spvcType.vectorSize, spvcType.matCol);

                case SPVC_BASETYPE_UINT8:
                    return ConvertBaseTypeToType(GL_UNSIGNED_BYTE, spvcType.vectorSize, spvcType.matCol);

                case SPVC_BASETYPE_UINT16:
                case SPVC_BASETYPE_UINT32:
                case SPVC_BASETYPE_UINT64:
                    return ConvertBaseTypeToType(GL_UNSIGNED_INT, spvcType.vectorSize, spvcType.matCol);

                case SPVC_BASETYPE_ATOMIC_COUNTER:
                    return GL_UNSIGNED_INT_ATOMIC_COUNTER;

                case SPVC_BASETYPE_FP16:
                case SPVC_BASETYPE_FP32:
                    return ConvertBaseTypeToType(GL_FLOAT, spvcType.vectorSize, spvcType.matCol);

                case SPVC_BASETYPE_FP64:
                    return ConvertBaseTypeToType(GL_DOUBLE, spvcType.vectorSize, spvcType.matCol);

                // FIXME: likely wrong conversions!
                case SPVC_BASETYPE_IMAGE:
                    return GL_IMAGE_2D;
                case SPVC_BASETYPE_SAMPLED_IMAGE:
                    return GL_IMAGE_2D;
                case SPVC_BASETYPE_SAMPLER:
                    return GL_SAMPLER_2D;

                case SPVC_BASETYPE_STRUCT:
                case SPVC_BASETYPE_VOID:
                case SPVC_BASETYPE_UNKNOWN:
                case SPVC_BASETYPE_ACCELERATION_STRUCTURE:
                default:
                    return GL_FALSE;
            }
        }
    }
}
