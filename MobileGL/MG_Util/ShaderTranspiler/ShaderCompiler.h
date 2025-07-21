//
// Created by Swung 0x48 on 2025/7/20.
//

#ifndef MOBILEGLTEST_SHADERCOMPILER_H
#define MOBILEGLTEST_SHADERCOMPILER_H

namespace MobileGL {
namespace MG_Util {
namespace ShaderTranspiler {
    class ShaderCompiler {
    public:
        static Result<SharedPtr<glslang::TShader>> CompileShader(const ShaderAttrib& attrib);
        static Result<Vector<Vector<unsigned>>> LinkProgram(const ProgramAttrib& attrib);
        static Result<String> DecompileShader(SpvcSession& session);
    };
}
}
}


#endif //MOBILEGLTEST_SHADERCOMPILER_H
