// MobileGL - MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ShaderSourceProcessor.h"
#include "ShaderSourceProcessor_inline.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            void PreprocessShaderSource(ShaderStage stage, String& source) {
                // remove multi-line comment
                size_t commentStartPos = source.find("/*");
                while (commentStartPos != String::npos) {
                    size_t commentEndPos = source.find("*/", commentStartPos);
                    // + length of "*/"
                    source = source.replace(commentStartPos, commentEndPos - commentStartPos + 2, "");
                    commentStartPos = source.find("/*", commentStartPos);
                }

                // remove #line directives
                SizeT linedirPos = source.find("#line");
                while (linedirPos != String::npos) {
                    SizeT newlinePos = source.find('\n', linedirPos);
                    // + length of "\n"
                    source = source.replace(linedirPos, newlinePos - linedirPos + 1, "");
                    linedirPos = source.find("#line", linedirPos);
                }

                // remove "noperspective"
                const char* str_np = "noperspective";
                const SizeT len_np = strlen(str_np);
                SizeT noperspectivePos = source.find(str_np);
                while (noperspectivePos != String::npos) {
                    // + length of "\n"
                    source = source.replace(noperspectivePos, len_np, "");
                    noperspectivePos = source.find(str_np);
                }

                // 注入 textureQueryLod 实现
                const char* str_textureQueryLod = "textureQueryLod";
                if (source.find(str_textureQueryLod) != std::string::npos && !std::getenv("LIBGL_ANGLE")) {
                    // 检查是否已经定义了 mg_textureQueryLod
                    const char* str_mg_textureQueryLod = "mg_textureQueryLod";
                    if (source.find(str_mg_textureQueryLod) == std::string::npos) {
                        // 检查是否已经有 #define textureQueryLod
                        const char* str_textureQueryLodDefine = "#define textureQueryLod";
                        if (source.find(str_textureQueryLodDefine) == std::string::npos) {
                            // 在顶部插入 textureQueryLod 实现
                            const char* textureQueryLodImpl = R"(
#define textureQueryLod mg_textureQueryLod

vec2 mg_textureQueryLod(sampler2D tex, vec2 uv) {
    vec2 texSizeF = vec2(textureSize(tex, 0));
    vec2 dFdx_uv = dFdx(uv * texSizeF);
    vec2 dFdy_uv = dFdy(uv * texSizeF);
    float maxDerivative = max(length(dFdx_uv), length(dFdy_uv));
    float lod = log2(maxDerivative);
    return vec2(lod);
}
)";
            
                            SizeT versionPos = source.find("#version");
                            if (versionPos != std::string::npos) {
                                SizeT lineEnd = source.find('\n', versionPos);
                                if (lineEnd != std::string::npos) {
                                    source = source.insert(lineEnd + 1, textureQueryLodImpl);
                                }
                            }
                        }
                    }
                }

                // 替换attribute关键字（顶点着色器）
                if (stage == ShaderStage::Vertex) {
                    const char* str_attribute = "attribute";
                    const SizeT len_attribute = strlen(str_attribute);
                    SizeT attributePos = source.find(str_attribute);
                    while (attributePos != String::npos) {
                        // 检查是否是独立的关键字（前后有空格或换行）
                        bool isKeyword = false;
                        if (attributePos > 0) {
                            char before = source[attributePos - 1];
                            if (before == ' ' || before == '\t' || before == '\n' || before == '\r') {
                                isKeyword = true;
                            }
                        } else {
                            isKeyword = true;
                        }
                        
                        if (isKeyword && attributePos + len_attribute < source.length()) {
                            char after = source[attributePos + len_attribute];
                            if (after == ' ' || after == '\t' || after == '\n' || after == '\r') {
                                source = source.replace(attributePos, len_attribute, "in");
                                attributePos = source.find(str_attribute, attributePos + 2); // 2 = strlen("in")
                                continue;
                            }
                        }
                        attributePos = source.find(str_attribute, attributePos + len_attribute);
                    }
                }

                // 替换varying关键字
                const char* str_varying = "varying";
                const SizeT len_varying = strlen(str_varying);
                SizeT varyingPos = source.find(str_varying);
                while (varyingPos != String::npos) {
                    bool isKeyword = false;
                    if (varyingPos > 0) {
                        char before = source[varyingPos - 1];
                        if (before == ' ' || before == '\t' || before == '\n' || before == '\r') {
                            isKeyword = true;
                        }
                    } else {
                        isKeyword = true;
                    }
                    
                    if (isKeyword && varyingPos + len_varying < source.length()) {
                        char after = source[varyingPos + len_varying];
                        if (after == ' ' || after == '\t' || after == '\n' || after == '\r') {
                            // 根据着色器阶段替换
                            if (stage == ShaderStage::Vertex) {
                                source = source.replace(varyingPos, len_varying, "out");
                                varyingPos = source.find(str_varying, varyingPos + 3); // 3 = strlen("out")
                            } else if (stage == ShaderStage::Fragment) {
                                source = source.replace(varyingPos, len_varying, "in");
                                varyingPos = source.find(str_varying, varyingPos + 2); // 2 = strlen("in")
                            }
                        }
                    }

                }

                inject_glsl_replacements(stage, source);
                
                // force #version
                ShaderProfile profile = ShaderProfile::Core;
                SizeT versionPos = source.find("#version");
                SizeT lineEnd = source.find('\n', versionPos);

                if (versionPos != String::npos) {
                    String versionLine = source.substr(versionPos, lineEnd - versionPos);

                    if (versionLine.find("ES") != String::npos)
                        profile = ShaderProfile::ES;
                    else if ((versionLine.find("compatibility") != String::npos) /*|| (versionLine.find("120") != String::npos) || (versionLine.find("130") != String::npos) || (versionLine.find("140") != String::npos) || (versionLine.find("100") != String::npos)*/)
                        profile = ShaderProfile::Compatibility;
                    else
                        profile = ShaderProfile::Core;
                } else {
                    profile = ShaderProfile::Core;
                    source.insert(0, "#version 460 compatibility\n");
                    return;
                }

                SizeT firstLineEnd = lineEnd;

                if (profile != ShaderProfile::ES) {
                    constexpr const char* versionDirectiveCore = "#version 460 compatibility\n";
                    constexpr const char* versionDirectiveCompat = "#version 460 compatibility\n";

                    const char* replacement =
                        (profile == ShaderProfile::Compatibility) ? versionDirectiveCompat : versionDirectiveCore;

                    if (firstLineEnd != String::npos) {
                        source.replace(versionPos, firstLineEnd - versionPos + 1, replacement);
                    } else {
                        source = replacement;
                    }
                }

            }

        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
