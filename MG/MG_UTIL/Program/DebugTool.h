//
// Created by BZLZHH on 2025/5/3.
//

#ifndef MOBILEGL_PROGRAM_DEBUGTOOL_H
#define MOBILEGL_PROGRAM_DEBUGTOOL_H
#include "../../Includes.h"

namespace MG_Util::Program {
    std::string GLTypeToString(GLenum type);
    std::string FormatUniformValue(const UniformValue& value);
    void DumpUniforms(const ProgramState& state, GLuint program);
    void DumpCurrentUniforms(const ProgramState& state);
    std::string CompileSPIRVToGLSL(std::vector<unsigned int> spirv, uint glslVersion, bool isES);
}

#endif //MOBILEGL_PROGRAM_DEBUGTOOL_H
