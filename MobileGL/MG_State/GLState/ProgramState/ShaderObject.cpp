// MobileGL - MobileGL/MG_State/GLState/ProgramState/ShaderObject.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ShaderObject.h"
#include <MG_Util/ShaderTranspiler/Types.h>
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>
#include <MG_Util/Converters/MGToGL/ProgramEnumConverter.h>
#include <MG_Util/ShaderTranspiler/ShaderSourceProcessor.h>
#include <MG_Util/ShaderTranspiler/glslang/UniformTraverser.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ShaderObject::SetShaderSource(const String& source) {
                m_source = source;
            }

            void ShaderObject::SetShaderSource(String&& source) {
                m_source = Move(source);
            }

            void ShaderObject::Compile() {
                using namespace MG_Util::ShaderTranspiler;
                MG_Util::ShaderTranspiler::PreprocessShaderSource(m_stage, m_source);

                // Compile for OpenGL here, so that we can do validation and link
                // like a real OpenGL driver at linking stage
                // Will compile for other backends later.
                ShaderAttrib attrib{.shaderType = MG_Util::ConvertShaderStageToGLEnum(m_stage),
                                    .sourceStr = m_source,
                                    .flags = ShaderCompileBits::CompileForOpenGL};

                auto result = ShaderCompiler::CompileShader(attrib);
                if (result) {
                    m_compileStatus = true;
                    m_shader = result.value();
                } else {
                    m_compileStatus = false;
                    m_infoLog = result.error().log;
                }
            }

            void ShaderObject::MarkAsDeleted() {
                m_deleteStatus = true;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL