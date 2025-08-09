#pragma once
#include <Includes.h>
#include "SpvcSession.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            class ShaderCompiler {
            public:
                static Result<SharedPtr<glslang::TShader>> CompileShader(const ShaderAttrib& attrib);
                static Result<Vector<Vector<unsigned>>> LinkProgram(const ProgramAttrib& attrib);
                static Result<String> DecompileShader(SpvcSession& session);
            };
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
