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

    struct FlattenedVaryingMember {
        const char* typeName;
        const char* memberName;
    };

    constexpr FlattenedVaryingMember kDailyWeatherVariationMembers[] = {
        {"vec2", "clouds_cumulus_coverage"},
        {"vec2", "clouds_altocumulus_coverage"},
        {"vec2", "clouds_cirrus_coverage"},
        {"float", "clouds_cumulus_congestus_amount"},
        {"float", "clouds_stratus_amount"},
        {"float", "fogginess"},
        {"float", "aurora_amount"},
        {"float", "nlc_amount"},
        {"mat2x3", "aurora_colors"},
    };

    bool IsIdentifierChar(char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
    }

    bool HasIdentifierBoundaries(const MobileGL::String& source, SizeT pos, SizeT length) {
        const bool hasLeftBoundary = pos == 0 || !IsIdentifierChar(source[pos - 1]);
        const SizeT end = pos + length;
        const bool hasRightBoundary = end >= source.size() || !IsIdentifierChar(source[end]);
        return hasLeftBoundary && hasRightBoundary;
    }

    SizeT FindToken(const MobileGL::String& source, const MobileGL::String& token, SizeT start = 0) {
        SizeT pos = start;
        while ((pos = source.find(token, pos)) != MobileGL::String::npos) {
            if (HasIdentifierBoundaries(source, pos, token.size())) {
                return pos;
            }
            pos += token.size();
        }
        return MobileGL::String::npos;
    }

    void ReplaceTokenOccurrencesInRange(MobileGL::String& source, SizeT rangeStart, SizeT rangeEnd,
                                        const MobileGL::String& from, const MobileGL::String& to) {
        SizeT pos = rangeStart;
        while ((pos = source.find(from, pos)) != MobileGL::String::npos && pos < rangeEnd) {
            if (!HasIdentifierBoundaries(source, pos, from.size())) {
                pos += from.size();
                continue;
            }

            source.replace(pos, from.size(), to);
            const auto delta = static_cast<std::ptrdiff_t>(to.size()) - static_cast<std::ptrdiff_t>(from.size());
            rangeEnd = static_cast<SizeT>(static_cast<std::ptrdiff_t>(rangeEnd) + delta);
            pos += to.size();
        }
    }

    void ReplaceAll(MobileGL::String& source, const MobileGL::String& from, const MobileGL::String& to) {
        SizeT pos = 0;
        while ((pos = source.find(from, pos)) != MobileGL::String::npos) {
            source.replace(pos, from.size(), to);
            pos += to.size();
        }
    }

    MobileGL::String TrimWhitespace(const MobileGL::String& input) {
        SizeT begin = 0;
        while (begin < input.size() && std::isspace(static_cast<unsigned char>(input[begin]))) {
            begin++;
        }

        SizeT end = input.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
            end--;
        }

        return input.substr(begin, end - begin);
    }

    bool FindFunctionBody(const MobileGL::String& source, const MobileGL::String& signature, SizeT* bodyStart,
                          SizeT* bodyEnd) {
        const SizeT signaturePos = source.find(signature);
        if (signaturePos == MobileGL::String::npos) {
            return false;
        }

        const SizeT bracePos = source.find('{', signaturePos + signature.size());
        if (bracePos == MobileGL::String::npos) {
            return false;
        }

        int depth = 1;
        for (SizeT pos = bracePos + 1; pos < source.size(); pos++) {
            if (source[pos] == '{') {
                depth++;
            } else if (source[pos] == '}') {
                depth--;
                if (depth == 0) {
                    *bodyStart = bracePos + 1;
                    *bodyEnd = pos;
                    return true;
                }
            }
        }

        return false;
    }

    void RenameDailyWeatherVariationHelperLocal(MobileGL::String& source) {
        constexpr const char* kHelperSignature = "DailyWeatherVariation get_daily_weather_variation()";
        constexpr const char* kInterfaceName = "daily_weather_variation";
        constexpr const char* kLocalName = "mg_daily_weather_variation_local";

        SizeT bodyStart = 0;
        SizeT bodyEnd = 0;
        if (!FindFunctionBody(source, kHelperSignature, &bodyStart, &bodyEnd)) {
            return;
        }

        ReplaceTokenOccurrencesInRange(source, bodyStart, bodyEnd, kInterfaceName, kLocalName);
    }

    bool RewriteDailyWeatherVariationInterface(MobileGL::ShaderStage stage, MobileGL::String& source) {
        using MobileGL::ShaderStage;

        if (stage != ShaderStage::Vertex && stage != ShaderStage::Fragment) {
            return false;
        }

        constexpr const char* kTypeName = "DailyWeatherVariation";
        constexpr const char* kInterfaceName = "daily_weather_variation";
        constexpr const char* kTempName = "mg_daily_weather_variation_tmp";
        const MobileGL::String declarationNeedle = MobileGL::String(kTypeName) + " " + kInterfaceName + ";";

        RenameDailyWeatherVariationHelperLocal(source);

        const SizeT declarationPos = source.find(declarationNeedle);
        if (declarationPos == MobileGL::String::npos) {
            return false;
        }

        SizeT lineStart = source.rfind('\n', declarationPos);
        lineStart = (lineStart == MobileGL::String::npos) ? 0 : lineStart + 1;
        SizeT lineEnd = source.find('\n', declarationPos);
        if (lineEnd == MobileGL::String::npos) {
            lineEnd = source.size();
        }

        const MobileGL::String declarationLine = source.substr(lineStart, lineEnd - lineStart);
        MOBILEGL_ASSERT(declarationLine.find("layout(") == MobileGL::String::npos,
                        "PreprocessShaderSource: unexpected explicit layout on DailyWeatherVariation interface in stage=%d",
                        static_cast<int>(stage));

        const bool hasInputQualifier = declarationLine.find(" in ") != MobileGL::String::npos ||
                                       declarationLine.rfind("in ", 0) == 0;
        const bool hasOutputQualifier = declarationLine.find(" out ") != MobileGL::String::npos ||
                                        declarationLine.rfind("out ", 0) == 0;
        MOBILEGL_ASSERT(hasInputQualifier != hasOutputQualifier,
                        "PreprocessShaderSource: expected a single in/out qualifier on DailyWeatherVariation interface in stage=%d line='%s'",
                        static_cast<int>(stage), declarationLine.c_str());

        const MobileGL::String qualifierPrefix = source.substr(lineStart, declarationPos - lineStart);
        MobileGL::String replacementDeclaration;
        for (const auto& member : kDailyWeatherVariationMembers) {
            replacementDeclaration += qualifierPrefix;
            replacementDeclaration += member.typeName;
            replacementDeclaration += " ";
            replacementDeclaration += kInterfaceName;
            replacementDeclaration += "_";
            replacementDeclaration += member.memberName;
            replacementDeclaration += ";\n";
        }
        source.replace(lineStart, lineEnd - lineStart + (lineEnd < source.size() ? 1 : 0), replacementDeclaration);

        SizeT assignPos = FindToken(source, kInterfaceName);
        while (assignPos != MobileGL::String::npos) {
            SizeT probe = assignPos + strlen(kInterfaceName);
            while (probe < source.size() && std::isspace(static_cast<unsigned char>(source[probe]))) {
                probe++;
            }

            if (probe >= source.size() || source[probe] != '=') {
                assignPos = FindToken(source, kInterfaceName, assignPos + strlen(kInterfaceName));
                continue;
            }

            SizeT statementStart = source.rfind('\n', assignPos);
            statementStart = (statementStart == MobileGL::String::npos) ? 0 : statementStart + 1;
            for (SizeT i = statementStart; i < assignPos; i++) {
                MOBILEGL_ASSERT(std::isspace(static_cast<unsigned char>(source[i])),
                                "PreprocessShaderSource: unexpected inline DailyWeatherVariation assignment in stage=%d",
                                static_cast<int>(stage));
            }

            const MobileGL::String indentation = source.substr(statementStart, assignPos - statementStart);
            const SizeT statementEnd = source.find(';', probe);
            MOBILEGL_ASSERT(statementEnd != MobileGL::String::npos,
                            "PreprocessShaderSource: missing ';' after DailyWeatherVariation assignment in stage=%d",
                            static_cast<int>(stage));

            const MobileGL::String rhsExpression = TrimWhitespace(source.substr(probe + 1, statementEnd - probe - 1));
            MobileGL::String replacementStatement;
            replacementStatement += indentation;
            replacementStatement += "{\n";
            replacementStatement += indentation;
            replacementStatement += "    DailyWeatherVariation ";
            replacementStatement += kTempName;
            replacementStatement += " = ";
            replacementStatement += rhsExpression;
            replacementStatement += ";\n";
            for (const auto& member : kDailyWeatherVariationMembers) {
                replacementStatement += indentation;
                replacementStatement += "    ";
                replacementStatement += kInterfaceName;
                replacementStatement += "_";
                replacementStatement += member.memberName;
                replacementStatement += " = ";
                replacementStatement += kTempName;
                replacementStatement += ".";
                replacementStatement += member.memberName;
                replacementStatement += ";\n";
            }
            replacementStatement += indentation;
            replacementStatement += "}";
            if (statementEnd + 1 < source.size() && source[statementEnd + 1] == '\n') {
                replacementStatement += "\n";
                source.replace(statementStart, statementEnd - statementStart + 2, replacementStatement);
            } else {
                source.replace(statementStart, statementEnd - statementStart + 1, replacementStatement);
            }

            assignPos = FindToken(source, kInterfaceName, statementStart + replacementStatement.size());
        }

        for (const auto& member : kDailyWeatherVariationMembers) {
            const MobileGL::String from = MobileGL::String(kInterfaceName) + "." + member.memberName;
            const MobileGL::String to = MobileGL::String(kInterfaceName) + "_" + member.memberName;
            ReplaceAll(source, from, to);
        }

        MOBILEGL_ASSERT(source.find(MobileGL::String(kTypeName) + " " + kInterfaceName + ";") == MobileGL::String::npos,
                        "PreprocessShaderSource: unrewritten DailyWeatherVariation interface declaration remained in stage=%d",
                        static_cast<int>(stage));
        MOBILEGL_ASSERT(source.find(MobileGL::String(kInterfaceName) + ".") == MobileGL::String::npos,
                        "PreprocessShaderSource: unrewritten DailyWeatherVariation member access remained in stage=%d",
                        static_cast<int>(stage));
        return true;
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

                // Some shader packs define helpers with built-in GLSL names such as round(), tanh(), or fma().
                // These may pass OpenGL-style validation but fail when recompiled for Vulkan/SPIR-V generation.
                RenameBuiltinShadowingFunction(source, "round", "mg_round");
                RenameBuiltinShadowingFunction(source, "tanh", "mg_tanh");
                RenameBuiltinShadowingFunction(source, "fma", "mg_fma");

                RewriteDailyWeatherVariationInterface(stage, source);
            }

        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
