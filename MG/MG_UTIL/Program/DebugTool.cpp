//
// Created by BZLZHH on 2025/5/3.
//

#include "DebugTool.h"

namespace MG_Util::Program {
    std::string GLTypeToString(GLenum type) {
        switch(type) {
            case GL_FLOAT: return "float";
            case GL_FLOAT_VEC2: return "vec2";
            case GL_FLOAT_VEC3: return "vec3";
            case GL_FLOAT_VEC4: return "vec4";
            case GL_INT: return "int";
            case GL_INT_VEC2: return "ivec2";
            case GL_INT_VEC3: return "ivec3";
            case GL_INT_VEC4: return "ivec4";
            case GL_UNSIGNED_INT: return "uint";
            case GL_UNSIGNED_INT_VEC2: return "uvec2";
            case GL_UNSIGNED_INT_VEC3: return "uvec3";
            case GL_UNSIGNED_INT_VEC4: return "uvec4";
            case GL_BOOL: return "bool";
            case GL_BOOL_VEC2: return "bvec2";
            case GL_BOOL_VEC3: return "bvec3";
            case GL_BOOL_VEC4: return "bvec4";
            case GL_FLOAT_MAT2: return "mat2";
            case GL_FLOAT_MAT3: return "mat3";
            case GL_FLOAT_MAT4: return "mat4";
            case GL_FLOAT_MAT2x3: return "mat2x3";
            case GL_FLOAT_MAT2x4: return "mat2x4";
            case GL_FLOAT_MAT3x2: return "mat3x2";
            case GL_FLOAT_MAT3x4: return "mat3x4";
            case GL_FLOAT_MAT4x2: return "mat4x2";
            case GL_FLOAT_MAT4x3: return "mat4x3";
            case GL_SAMPLER_1D: return "sampler1D";
            case GL_SAMPLER_2D: return "sampler2D";
            case GL_SAMPLER_3D: return "sampler3D";
            case GL_SAMPLER_CUBE: return "samplerCube";
            case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow";
            case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
            case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
            case GL_SAMPLER_1D_ARRAY: return "sampler1DArray";
            case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
            case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow";
            case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
            case GL_SAMPLER_BUFFER: return "samplerBuffer";
            case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS";
            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
            default: return "UnknownType(0x" + std::to_string(type) + ")";
        }
    }
    
    std::string FormatUniformValue(const UniformValue& value) {
        std::ostringstream ss;
        ss << "[";

        const size_t elemCount = std::min<size_t>(value.numElements, 16);

        switch(value.type) {
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
                for(size_t i=0; i<elemCount; ++i) {
                    if (i > 0) ss << ", ";
                    ss << (i < value.floatData.size() ? value.floatData[i] : 0.0f);
                }
                break;

            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
                for(size_t i=0; i<elemCount; ++i) {
                    if (i > 0) ss << ", ";
                    ss << (i < value.intData.size() ? value.intData[i] : 0);
                }
                break;

            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_INT_VEC2:
            case GL_UNSIGNED_INT_VEC3:
            case GL_UNSIGNED_INT_VEC4:
                for(size_t i=0; i<elemCount; ++i) {
                    if (i > 0) ss << ", ";
                    ss << (i < value.uintData.size() ? value.uintData[i] : 0);
                }
                break;

            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
                for(size_t i=0; i<elemCount; ++i) {
                    if (i > 0) ss << ", ";
                    ss << (i < value.boolData.size() ? (value.boolData[i] ? "true" : "false") : "false");
                }
                break;

            default:
                ss << "N/A";
        }

        if (value.count > elemCount) ss << ", ...";
        ss << "]";
        return ss.str();
    }
    
    void DumpUniforms(const ProgramState& state, GLuint program) {
        if constexpr (MG_Global::Common::LogLevel > MG_Constants::Common::LOG_LEVEL_DEBUG)
            return;
        
        auto* prog = state.GetProgramObject(program);
        if (!prog->linked.toBool()) {
            MG_Util::Debug::LogE("Program %u not linked", program);
            return;
        }
        MG_Util::Debug::LogD("=== Dumping uniforms for program %u ===", program);
        for(const auto& [name, loc] : prog->uniformLocations) {
            const auto& value = prog->uniformValues.at(name);
            MG_Util::Debug::LogD("Uniform: %-24s Location: %-4d Type: %-16s Count: %-3d Value: %s",
                                 name.c_str(),
                                 loc,
                                 GLTypeToString(value.type).c_str(),
                                 value.count,
                                 FormatUniformValue(value).c_str());
        }
    }
    
    void DumpCurrentUniforms(const ProgramState& state) {
        if (MG_Global::Common::LogLevel > MG_Constants::Common::LOG_LEVEL_DEBUG)
            return;
        
        GLuint current = state.GetCurrentProgram();
        if (current == 0) {
            return;
        }
        DumpUniforms(state, current);
    }
}