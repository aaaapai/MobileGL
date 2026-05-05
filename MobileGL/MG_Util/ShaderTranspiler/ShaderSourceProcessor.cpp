// MobileGL - MobileGL/MG_Util/ShaderTranspiler/ShaderSourceProcessor.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "ShaderSourceProcessor.h"

#include <cctype>

namespace {
    using MobileGL::SizeT;

    bool IsIdentifierChar(char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
    }

    bool HasSingleLineFunctionDefinition(const MobileGL::String& source, const MobileGL::String& functionName) {
        SizeT lineStart = 0;
        while (lineStart < source.size()) {
            SizeT lineEnd = source.find('\n', lineStart);
            if (lineEnd == MobileGL::String::npos) {
                lineEnd = source.size();
            }

            SizeT functionPos = source.find(functionName, lineStart);
            while (functionPos != MobileGL::String::npos && functionPos < lineEnd) {
                const bool hasLeftBoundary = functionPos == 0 || !IsIdentifierChar(source[functionPos - 1]);
                const SizeT functionEnd = functionPos + functionName.size();
                const bool hasRightBoundary =
                    functionEnd >= source.size() || !IsIdentifierChar(source[functionEnd]);
                if (hasLeftBoundary && hasRightBoundary) {
                    SizeT probe = functionEnd;
                    while (probe < lineEnd && std::isspace(static_cast<unsigned char>(source[probe]))) {
                        probe++;
                    }
                    if (probe < lineEnd && source[probe] == '(') {
                        const SizeT closingParen = source.find(')', probe);
                        if (closingParen != MobileGL::String::npos && closingParen < lineEnd) {
                            probe = closingParen + 1;
                            while (probe < lineEnd && std::isspace(static_cast<unsigned char>(source[probe]))) {
                                probe++;
                            }
                            if (probe < lineEnd && source[probe] == '{') {
                                return true;
                            }
                        }
                    }
                }

                functionPos = source.find(functionName, functionPos + functionName.size());
            }

            lineStart = lineEnd + 1;
        }

        return false;
    }

    void RenameFunctionInvocations(MobileGL::String& source, const MobileGL::String& from, const MobileGL::String& to) {
        SizeT pos = 0;
        while ((pos = source.find(from, pos)) != MobileGL::String::npos) {
            const bool hasLeftBoundary = pos == 0 || !IsIdentifierChar(source[pos - 1]);
            const SizeT end = pos + from.size();
            const bool hasRightBoundary = end >= source.size() || !IsIdentifierChar(source[end]);

            SizeT probe = end;
            while (probe < source.size() && std::isspace(static_cast<unsigned char>(source[probe]))) {
                probe++;
            }

            if (hasLeftBoundary && hasRightBoundary && probe < source.size() && source[probe] == '(') {
                source.replace(pos, from.size(), to);
                pos += to.size();
                continue;
            }

            pos = end;
        }
    }

    void RenameBuiltinShadowingFunction(MobileGL::String& source, const char* from, const char* to) {
        const MobileGL::String fromName = from;
        if (!HasSingleLineFunctionDefinition(source, fromName)) {
            return;
        }

        RenameFunctionInvocations(source, fromName, to);
    }
} // namespace

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
                    source.insert(0, "#version 460 core\n");
                    return;
                }

                SizeT firstLineEnd = lineEnd;

                if (profile != ShaderProfile::ES) {
                    constexpr const char* versionDirectiveCore = "#version 460 core\n";
                    constexpr const char* versionDirectiveCompat = "#version 460 compatibility\n";

                    const char* replacement =
                        (profile == ShaderProfile::Compatibility) ? versionDirectiveCompat : versionDirectiveCore;

                    if (firstLineEnd != String::npos) {
                        source.replace(versionPos, firstLineEnd - versionPos + 1, replacement);
                    } else {
                        source = replacement;
                    }
                }

                // Some shader packs define helpers with built-in GLSL names such as round().
                // These may pass OpenGL-style validation but fail when recompiled for Vulkan/SPIR-V generation.
                RenameBuiltinShadowingFunction(source, "round", "mg_round");
            }

        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
