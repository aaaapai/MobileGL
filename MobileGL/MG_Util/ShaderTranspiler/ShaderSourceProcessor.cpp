#include "ShaderSourceProcessor.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            void PreprocessShaderSource(ShaderStage stage, String& source) {
                // remove multi-line comment
                size_t commentStartPos = source.find("/*");
                while (commentStartPos != std::string::npos) {
                    size_t commentEndPos = source.find("*/", commentStartPos);
                    // + length of "*/"
                    source = source.replace(commentStartPos, commentEndPos - commentStartPos + 2, "");
                    commentStartPos = source.find("/*", commentStartPos);
                }

                // remove #line directives
                SizeT linedirPos = source.find("#line");
                while (linedirPos != std::string::npos) {
                    SizeT newlinePos = source.find('\n', linedirPos);
                    // + length of "\n"
                    source = source.replace(linedirPos, newlinePos - linedirPos + 1, "");
                    linedirPos = source.find("#line", linedirPos);
                }

                // remove "noperspective"
                const char* str_np = "noperspective";
                const SizeT len_np = strlen(str_np);
                SizeT noperspectivePos = source.find(str_np);
                while (noperspectivePos != std::string::npos) {
                    // + length of "\n"
                    source = source.replace(noperspectivePos, len_np, "");
                    noperspectivePos = source.find(str_np);
                }

                // force #version
                ShaderProfile profile = ShaderProfile::Core;
                SizeT versionPos = source.find("#version");

                if (versionPos != String::npos) {
                    SizeT lineEnd = source.find('\n', versionPos);
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

                SizeT firstLineEnd = source.find('\n');

                if (profile != ShaderProfile::ES) {
                    constexpr const char* versionDirectiveCore = "#version 460 core\n";
                    constexpr const char* versionDirectiveCompat = "#version 460 compatibility\n";

                    const char* replacement =
                        (profile == ShaderProfile::Compatibility) ? versionDirectiveCompat : versionDirectiveCore;

                    if (firstLineEnd != std::string::npos) {
                        source.replace(0, firstLineEnd + 1, replacement);
                    } else {
                        source = replacement;
                    }
                }
            }

        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
