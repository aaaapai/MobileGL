// MobileGL - MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "ShaderSourceProcessor.h"

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
                while (noperspectivePos != String::npos && !std::getenv("LIBGL_ANGLE")) { //允许ANGLE支持？
                    // + length of "\n"
                    source = source.replace(noperspectivePos, len_np, "");
                    noperspectivePos = source.find(str_np);
                }

                // 处理 GLSL 代码中的特殊关键字和函数

                // 处理 isamplerBuffer 相关代码
                const char* str_isamplerBuffer = "isamplerBuffer";
                const SizeT len_isamplerBuffer = strlen(str_isamplerBuffer);
                if (source.find(str_isamplerBuffer) != std::string::npos) {
                    SizeT isamplerBufferPos = source.find(str_isamplerBuffer);
                    while (isamplerBufferPos != std::string::npos) {
                        source = source.replace(isamplerBufferPos, len_isamplerBuffer, "isampler2D");
                        isamplerBufferPos = source.find(str_isamplerBuffer, isamplerBufferPos + 11);
                    }
    
                    // 处理 texelFetch 调用格式
                    const char* str_texelFetch = "texelFetch";
                    SizeT texelFetchPos = source.find(str_texelFetch);
                    while (texelFetchPos != std::string::npos) {
                        // 查找括号开始
                        SizeT parenStart = source.find('(', texelFetchPos);
                        if (parenStart == std::string::npos) {
                            break;
                        }
        
                        // 查找括号结束
                        int parenCount = 1;
                        SizeT parenEnd = parenStart + 1;
                        while (parenEnd < source.length() && parenCount > 0) {
                            if (source[parenEnd] == '(') parenCount++;
                            else if (source[parenEnd] == ')') parenCount--;
                            parenEnd++;
                        }
        
                        if (parenCount == 0) {
                            std::string oldCall = source.substr(texelFetchPos, parenEnd - texelFetchPos);
                            std::string params = source.substr(parenStart + 1, parenEnd - parenStart - 2);
            
                            // 移除空白字符
                            params.erase(std::remove_if(params.begin(), params.end(), ::isspace), params.end());
                            
                            // 分割参数
                            SizeT commaPos = params.find(',');
                            if (commaPos != std::string::npos) {
                                std::string samplerName = params.substr(0, commaPos);
                                std::string index = params.substr(commaPos + 1);
                
                                // 构建新的调用
                                std::string newCall = "texelFetch(" + samplerName + 
                                                      ", ivec2((" + index + ") % u_BufferTexWidth, (" + 
                                                      index + ") / u_BufferTexWidth), 0)";
                
                                source = source.replace(texelFetchPos, parenEnd - texelFetchPos, newCall);
                                texelFetchPos = source.find(str_texelFetch, texelFetchPos + newCall.length());
                            } else {
                                texelFetchPos = source.find(str_texelFetch, texelFetchPos + 1);
                            }
                        } else {
                            texelFetchPos = source.find(str_texelFetch, texelFetchPos + 1);
                        }
                    }
    
                    // 查找合适的位置插入边界保护函数
                    const char* boundaryProtection = R"(
ivec2 bufferCoords(int index) {
    int width = u_BufferTexWidth;
    int x = index % width;
    int y = index / width;
    if (y >= u_BufferTexHeight) {
        y = u_BufferTexHeight - 1;
        x = width - 1;
    }
    return ivec2(x, y);
}
)";
    
                    // 在第一个函数定义前插入
                    SizeT insertPos = source.find("void main()");
                    if (insertPos == std::string::npos) {
                        insertPos = source.find("float ");
                        if (insertPos == std::string::npos) {
                            insertPos = source.find("vec");
                            if (insertPos == std::string::npos) {
                                insertPos = 0;
                            }
                        }
                    }
    
                    // 回退到前一个换行符
                    while (insertPos > 0 && source[insertPos - 1] != '\n') {
                        insertPos--;
                    }
    
                    if (insertPos != std::string::npos) {
                        source = source.insert(insertPos, boundaryProtection);
                    }
    
                    // 插入 uniform 声明
                    const char* uniformDecl = R"(
uniform int u_BufferTexWidth;
uniform int u_BufferTexHeight;
)";
    
                    // 在顶部插入 uniform 声明
                    SizeT versionPos = source.find("#version");
                    if (versionPos != std::string::npos) {
                        SizeT lineEnd = source.find('\n', versionPos);
                        if (lineEnd != std::string::npos) {
                            source = source.insert(lineEnd + 1, uniformDecl);
                        }
                    }
                }

                // 注入 textureQueryLod 实现
                const char* str_textureQueryLod = "textureQueryLod";
                if (source.find(str_textureQueryLod) != std::string::npos) {
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
                
                // force #version
                ShaderProfile profile = ShaderProfile::Core;
                SizeT versionPos = source.find("#version");
                SizeT lineEnd = source.find('\n', versionPos);

                if (versionPos != String::npos) {
                    String versionLine = source.substr(versionPos, lineEnd - versionPos);

                    if (versionLine.find("ES") != String::npos)
                        profile = ShaderProfile::ES;
                    else if (versionLine.find("compatibility") != String::npos)
                        profile = ShaderProfile::Compatibility;
                    else
                        profile = ShaderProfile::Core;
                } else {
                    profile = ShaderProfile::Core;
                    source.insert(0, "#version 450 core\n");
                    return;
                }

                SizeT firstLineEnd = lineEnd;

                if (profile != ShaderProfile::ES) {
                    constexpr const char* versionDirectiveCore = "#version 450 core\n";
                    constexpr const char* versionDirectiveCompat = "#version 450 compatibility\n";

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
