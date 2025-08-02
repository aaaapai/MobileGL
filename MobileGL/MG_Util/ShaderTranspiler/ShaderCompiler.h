#pragma once

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    class ShaderCompiler {
    public:
        static Result<SharedPtr<glslang::TShader>> CompileShader(const ShaderAttrib& attrib);
        static Result<Vector<Vector<unsigned>>> LinkProgram(const ProgramAttrib& attrib);
        static Result<String> DecompileShader(SpvcSession& session);
        static Result<String> DecompileShader(spirv_cross::CompilerGLSL& compiler);
    };
}
}
}
