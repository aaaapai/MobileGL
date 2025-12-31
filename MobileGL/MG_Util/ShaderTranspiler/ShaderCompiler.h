// MobileGL - MobileGL/MG_Util/ShaderTranspiler/ShaderCompiler.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "SpvcSession.h"
#include "glslang/TVarEntryInfo.h"
#include "glslang/TMglGlslIoResolver.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            class ShaderCompiler {
            public:
                static Result<SharedPtr<glslang::TShader>> CompileShader(const ShaderAttrib& attrib);
                static Result<SharedPtr<glslang::TProgram>> LinkProgram(const ProgramAttrib& attrib);
                static Result<Vector<Vector<unsigned>>> GetSpirvBinaryFromProgram(const ProgramBinaryAttrib& attrib);
                static Result<String> DecompileShader(SpvcSession& session);
            };
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
