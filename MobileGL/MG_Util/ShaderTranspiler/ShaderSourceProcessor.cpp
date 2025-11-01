#include "ShaderSourceProcessor.h"

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            void PreprocessShaderSource(ShaderStage stage, String& source) {
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
