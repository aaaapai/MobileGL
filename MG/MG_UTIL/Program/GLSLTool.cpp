//
// Created by BZLZHH on 2025/5/3.
//

#include "../../Includes.h"

namespace MG_Util::Program {
    void RenameGLSLBuiltinsForVulkan(std::string &src) {
//        static const std::vector<std::pair<std::regex, std::string>> rules = {
//                { std::regex(R"(gl_VertexID)"),   "gl_VertexIndex"    },
//                { std::regex(R"(gl_InstanceID)"), "gl_InstanceIndex"  }
//        };
//
//        for (auto &rule : rules) {
//            src = std::regex_replace(src, rule.first, rule.second);
//        }
        size_t pos = src.find("gl_VertexID");
        if (pos != std::string::npos) {
            src.replace(pos, strlen("gl_VertexID"), "gl_VertexIndex");
        }

        pos = src.find("gl_InstanceID");
        if (pos != std::string::npos) {
            src.replace(pos, strlen("gl_InstanceID"), "gl_InstanceIndex");
        }
    }
    
    std::string BindInputLayoutLocationsForGLSL(
        std::vector<uint32_t>& spirv,
        MG_Global::unordered_map<std::string, GLint>& name_location_map)
    {
        spvc_context context = nullptr;
        spvc_parsed_ir ir = nullptr;
        spvc_compiler compiler = nullptr;
        spvc_resources resources = nullptr;
        spvc_result result = SPVC_SUCCESS;
        const char* glsl_source = nullptr;
        std::string output_glsl;

        if ((result = spvc_context_create(&context)) != SPVC_SUCCESS)
        {
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to create context.");
            return {};
        }

        if ((result = spvc_context_parse_spirv(context,
            spirv.data(),
            spirv.size(),
            &ir)) != SPVC_SUCCESS)
        {
            spvc_context_destroy(context);
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to parse SPIR-V.");
            return {};
        }

        if ((result = spvc_context_create_compiler(
            context,
            SPVC_BACKEND_GLSL,
            ir,
            SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
            &compiler)) != SPVC_SUCCESS)
        {
            spvc_context_destroy(context);
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to create compiler.");
            return {};
        }

        if ((result = spvc_compiler_create_shader_resources(compiler, &resources)) != SPVC_SUCCESS)
        {
            spvc_context_destroy(context);
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to create shader resources.");
            return {};
        }

        const spvc_reflected_resource* inputs = nullptr;
        size_t num_inputs = 0;
        if ((result = spvc_resources_get_resource_list_for_type(
            resources,
            SPVC_RESOURCE_TYPE_STAGE_INPUT,
            &inputs,
            &num_inputs)) != SPVC_SUCCESS)
        {
            spvc_context_destroy(context);
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to get stage inputs.");
            return {};
        }

        std::unordered_map<std::string, spvc_variable_id> name_to_id;
        for (size_t i = 0; i < num_inputs; ++i)
        {
            name_to_id[inputs[i].name] = inputs[i].id;
        }

        std::unordered_set<GLint> used_locations;

        for (auto& kv : name_location_map)
        {
            const auto& name = kv.first;
            GLint loc = kv.second;
            auto it = name_to_id.find(name);
            if (it == name_to_id.end())
            {
                MG_Util::Debug::LogW("[SPIRV-Cross] Input name not found: %s", name.c_str());
                continue;
            }

            spvc_compiler_set_decoration(
                compiler,
                it->second,
                SpvDecorationLocation,
                loc);

            used_locations.insert(loc);
        }

        GLint next_free_loc = 0;
        for (size_t i = 0; i < num_inputs; ++i)
        {
            auto& res = inputs[i];
            const auto& name = res.name;
            auto var_id = res.id;

            if (name_location_map.find(name) != name_location_map.end())
                continue;

            uint32_t curr_loc = 0;
            bool has_loc = false;
            curr_loc = spvc_compiler_get_decoration(compiler, var_id, SpvDecorationLocation);
            if (spvc_compiler_has_decoration(compiler, var_id, SpvDecorationLocation))
                has_loc = true;

            if (has_loc && !used_locations.contains(curr_loc))
            {
                used_locations.insert((GLint)curr_loc);
                continue;
            }

            while (used_locations.count(next_free_loc))
                ++next_free_loc;

            spvc_compiler_set_decoration(
                compiler,
                var_id,
                SpvDecorationLocation,
                next_free_loc);

            MG_Util::Debug::LogI("[SPIRV-Cross] Auto assigned '%s' to location %d", name, next_free_loc);
            used_locations.insert(next_free_loc);
        }

        spvc_compiler_options options = nullptr;
        spvc_compiler_create_compiler_options(compiler, &options);
        spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 450);
        spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_FALSE);
        spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_FLIP_VERTEX_Y, SPVC_TRUE);
        spvc_compiler_install_compiler_options(compiler, options);

        if ((result = spvc_compiler_compile(compiler, &glsl_source)) != SPVC_SUCCESS)
        {
            MG_Util::Debug::LogE("[SPIRV-Cross] Failed to compile: %s",
                spvc_context_get_last_error_string(context));
            spvc_context_destroy(context);
            return {};
        }

        output_glsl = glsl_source;
        spvc_context_destroy(context);
        MG_Util::Debug::LogI("[SPIRV-Cross] GLSL output:\n%s", output_glsl.c_str());

        size_t pos = output_glsl.find("\n    gl_Position.y = -gl_Position.y;\n}");
        if (pos != std::string::npos)
        {
            output_glsl.replace(
                pos,
                strlen("\n    gl_Position.y = -gl_Position.y;\n}"),
                "\n    gl_Position.y = -gl_Position.y;\n    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n}");
        }

        return output_glsl;
    }

    
    static std::vector<bool> buildCommentMask(const std::string &src) {
        enum State {
            NORMAL,
            LINE_COMMENT,
            BLOCK_COMMENT,
            STRING_LITERAL,
            CHAR_LITERAL
        };

        std::vector<bool> isComment(src.size(), false);
        State state = NORMAL;

        for (size_t i = 0; i < src.size(); ++i) {
            switch (state) {
                case NORMAL:
                    if (i + 1 < src.size() && src[i] == '/' && src[i + 1] == '/') {
                        state = LINE_COMMENT;
                        isComment[i] = true;
                        isComment[i + 1] = true;
                        ++i;
                    }
                    else if (i + 1 < src.size() && src[i] == '/' && src[i + 1] == '*') {
                        state = BLOCK_COMMENT;
                        isComment[i] = true;
                        isComment[i + 1] = true;
                        ++i;
                    }
                    else if (src[i] == '"') {
                        state = STRING_LITERAL;
                    }
                    else if (src[i] == '\'') {
                        state = CHAR_LITERAL;
                    }
                    break;

                case LINE_COMMENT:
                    isComment[i] = true;
                    if (src[i] == '\n') {
                        state = NORMAL;
                    }
                    break;

                case BLOCK_COMMENT:
                    isComment[i] = true;
                    if (i + 1 < src.size() && src[i] == '*' && src[i + 1] == '/') {
                        isComment[i + 1] = true;
                        state = NORMAL;
                        ++i;
                    }
                    break;

                case STRING_LITERAL:
                    if (src[i] == '\\' && (i + 1 < src.size())) {
                        i++;
                    }
                    else if (src[i] == '"') {
                        state = NORMAL;
                    }
                    break;

                case CHAR_LITERAL:
                    if (src[i] == '\\' && (i + 1 < src.size())) {
                        i++;
                    }
                    else if (src[i] == '\'') {
                        state = NORMAL;
                    }
                    break;
            }
        }

        return isComment;
    }

    static bool isPreprocessorLine(const std::string &src, size_t pos) {
        size_t lineStart = src.rfind('\n', pos);
        if (lineStart == std::string::npos) {
            lineStart = 0;
        } else {
            lineStart += 1;
        }
        size_t firstNonSpace = src.find_first_not_of(" \t\r\n", lineStart);
        if (firstNonSpace != std::string::npos && src[firstNonSpace] == '#') {
            return true;
        }
        return false;
    }

    static inline bool isInComment(const std::vector<bool> &mask, size_t pos) {
        if (pos >= mask.size()) return false;
        return mask[pos];
    }

    static inline void leftTrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }
    static inline void rightTrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
    static inline void trim(std::string &s) {
        leftTrim(s);
        rightTrim(s);
    }
    static size_t findNextNonSpace(const std::string& str, size_t pos) {
        return str.find_first_not_of(" \t\r\n", pos);
    }

    static std::string extractVarNameFromUniformDecl(const std::string& decl) {
        const std::regex layoutRegex(R"(\s*layout\s*\([^)]*\))");
        std::string declNoLayout = std::regex_replace(decl, layoutRegex, "");

        size_t uniformPos = declNoLayout.find("uniform");
        if (uniformPos != std::string::npos) {
            declNoLayout = declNoLayout.substr(uniformPos + 7);
        }
        trim(declNoLayout);

        size_t semiPos = declNoLayout.rfind(';');
        if (semiPos != std::string::npos) {
            declNoLayout = declNoLayout.substr(0, semiPos);
        }
        trim(declNoLayout);

        size_t lastSpace = declNoLayout.find_last_of(" \t\r\n");
        if (lastSpace == std::string::npos) {
            return declNoLayout;
        }

        std::string candidate = declNoLayout.substr(lastSpace + 1);
        size_t bracketPos = candidate.find('[');
        if (bracketPos != std::string::npos) {
            candidate = candidate.substr(0, bracketPos);
        }

        return candidate;
    }

    std::pair<std::string, std::string> GenerateDefaultUBOForGLSL(const std::pair<std::string,
                                                                  std::string>& glslSources) {
        std::string source1 = glslSources.first;
        std::string source2 = glslSources.second;

        std::vector<bool> commentMask1 = buildCommentMask(source1);
        std::vector<bool> commentMask2 = buildCommentMask(source2);

        const std::regex layoutRegex(R"(\s*layout\s*\([^)]*\))");
        std::set<std::string> allUniformNames;
        std::vector<std::string> mergedUniforms;
        std::vector<std::pair<size_t, size_t>> removalSpans1;
        std::vector<std::pair<size_t, size_t>> removalSpans2;

        auto processSource = [&](std::string& source, const std::vector<bool>& commentMask,
                                 std::vector<std::pair<size_t, size_t>>& removalSpans,
                                 bool isFirstSource)
        {
            size_t searchPos = 0;
            while ((searchPos = source.find("uniform", searchPos)) != std::string::npos) {
                if (isInComment(commentMask, searchPos)) {
                    searchPos++;
                    continue;
                }

                if (isPreprocessorLine(source, searchPos)) {
                    searchPos++;
                    continue;
                }

                if (searchPos > 0 && (std::isalnum(source[searchPos - 1]) || source[searchPos - 1] == '_')) {
                    searchPos++;
                    continue;
                }

                size_t nextCharPos = findNextNonSpace(source, searchPos + 7);
                if (nextCharPos == std::string::npos) break;

                if (source[nextCharPos] == '{') {
                    searchPos = nextCharPos + 1;
                    continue;
                }

                size_t bracePos = source.find('{', nextCharPos);
                size_t semicolonPos = source.find(';', nextCharPos);
                if (bracePos != std::string::npos && (semicolonPos == std::string::npos || bracePos < semicolonPos)) {
                    searchPos = bracePos + 1;
                    continue;
                }

                if (semicolonPos == std::string::npos) {
                    searchPos++;
                    continue;
                }

                size_t declStart = searchPos;
                size_t declLength = semicolonPos - searchPos + 1;
                std::string declaration = source.substr(declStart, declLength);

                std::string declNoLayout = std::regex_replace(declaration, layoutRegex, "");
                std::string afterUniform = declNoLayout.substr(declNoLayout.find("uniform") + 7);
                trim(afterUniform);
                size_t sep = afterUniform.find_first_of(" \t\r\n[");
                std::string typeToken = (sep == std::string::npos ? afterUniform : afterUniform.substr(0, sep));
                bool isSamplerOrImage = false;
                if (typeToken.rfind("sampler", 0) == 0 ||
                    typeToken.rfind("isampler", 0) == 0 ||
                    typeToken.rfind("usampler", 0) == 0 ||
                    typeToken.rfind("image", 0) == 0)
                {
                    isSamplerOrImage = true;
                }
                if (isSamplerOrImage) {
                    searchPos = declStart + declLength;
                    continue;
                }

                std::string varName = extractVarNameFromUniformDecl(declaration);

                if (isFirstSource) {
                    mergedUniforms.push_back(declaration);
                    allUniformNames.insert(varName);
                } else {
                    if (allUniformNames.find(varName) == allUniformNames.end()) {
                        mergedUniforms.push_back(declaration);
                        allUniformNames.insert(varName);
                    }
                }

                size_t removeStart = declStart;
                {
                    size_t scanBack = declStart;
                    while (scanBack > 0 && std::isspace((unsigned char)source[scanBack - 1])) {
                        scanBack--;
                    }
                    if (scanBack >= 6) {
                        size_t maybeLayout = source.rfind("layout", scanBack - 6);
                        if (maybeLayout != std::string::npos) {
                            size_t parenOpen = source.find('(', maybeLayout + 6);
                            if (parenOpen != std::string::npos && parenOpen < scanBack) {
                                size_t parenClose = source.find(')', parenOpen);
                                if (parenClose != std::string::npos && parenClose < declStart) {
                                    std::string between = source.substr(maybeLayout, declStart - maybeLayout);
                                    const std::regex onlyLayout(R"(layout\s*\([^)]*\)\s*)");
                                    if (std::regex_match(between, onlyLayout)) {
                                        removeStart = maybeLayout;
                                    }
                                }
                            }
                        }
                    }
                }
                removalSpans.emplace_back(removeStart, (declStart + declLength) - removeStart);

                searchPos = declStart + declLength;
            }
        };

        processSource(source1, commentMask1, removalSpans1, true);
        processSource(source2, commentMask2, removalSpans2, false);

        if (mergedUniforms.empty()) {
            return glslSources;
        }

        std::sort(removalSpans1.rbegin(), removalSpans1.rend());
        for (auto &span : removalSpans1) {
            source1.erase(span.first, span.second);
        }
        source1 = std::regex_replace(source1, std::regex(R"((\r\n|\n|\r){2,})"), "$1");

        std::sort(removalSpans2.rbegin(), removalSpans2.rend());
        for (auto &span : removalSpans2) {
            source2.erase(span.first, span.second);
        }
        source2 = std::regex_replace(source2, std::regex(R"((\r\n|\n|\r){2,})"), "$1");

        std::string uboBlock = "\nlayout(std140) uniform MG_DEFAULT_UBO\n{\n";
        for (auto &decl : mergedUniforms) {
            std::string cleaned = std::regex_replace(decl, layoutRegex, "");
            std::string memberDecl = cleaned.substr(cleaned.find("uniform") + 7);
            size_t semiPos = memberDecl.rfind(';');
            if (semiPos != std::string::npos) {
                memberDecl.erase(semiPos);
            }
            trim(memberDecl);
            uboBlock += "    " + memberDecl + ";\n";
        }
        uboBlock += "};\n";

        auto insertUBO = [&](std::string& source) {
            size_t insertPos = 0, lastDirectivePos = 0;
            size_t searchPos = 0;
            while ((searchPos = source.find('#', searchPos)) != std::string::npos) {
                if (isInComment(commentMask1, searchPos)) {
                    searchPos++;
                    continue;
                }

                size_t eol = source.find('\n', searchPos);
                if (eol == std::string::npos) break;
                std::string line = source.substr(searchPos, eol - searchPos);
                if (line.find("#version") != std::string::npos || line.find("#extension") != std::string::npos) {
                    lastDirectivePos = eol + 1;
                }
                searchPos = eol + 1;
            }
            insertPos = lastDirectivePos;
            source.insert(insertPos, uboBlock);
        };

        insertUBO(source1);
        insertUBO(source2);

        return {source1, source2};
    }

    void GenerateDefaultUBOForGLSL_Multi(
            MG_Global::unordered_map<GLuint, std::string> &shaderSources, std::vector<std::string>& outUniformBufferNames) {
        if (shaderSources.empty()) return;

        outUniformBufferNames.clear();

        std::unordered_map<GLuint, std::vector<bool>> commentMasks;
        for (auto const& [id, source] : shaderSources) {
            commentMasks[id] = buildCommentMask(source);
        }

        const std::regex layoutRegex(R"(\s*layout\s*\([^)]*\))");
        std::set<std::string> allUniformNames;
        std::vector<std::string> mergedUniforms;
        std::unordered_map<GLuint, std::vector<std::pair<size_t, size_t>>> removalSpansPerShader;

        for (auto& [id, source] : shaderSources) {
            std::vector<bool>& commentMask = commentMasks[id];
            // Ensure an entry exists for this shader ID, even if no uniforms are found
            auto& removalSpans = removalSpansPerShader[id];

            size_t searchPos = 0;
            while ((searchPos = source.find("uniform", searchPos)) != std::string::npos) {
                if (isInComment(commentMask, searchPos)) {
                    searchPos++;
                    continue;
                }

                if (isPreprocessorLine(source, searchPos)) {
                    searchPos++;
                    continue;
                }

                if (searchPos > 0 && (std::isalnum(source[searchPos - 1]) ||
                                      source[searchPos - 1] == '_')) {
                    searchPos++;
                    continue;
                }

                size_t nextCharPos = findNextNonSpace(source, searchPos + 7);
                if (nextCharPos == std::string::npos) break;

                if (source[nextCharPos] == '{') {
                    searchPos = nextCharPos + 1;
                    continue;
                }

                size_t bracePos = source.find('{', nextCharPos);
                size_t semicolonPos = source.find(';', nextCharPos);
                if (bracePos != std::string::npos &&
                    (semicolonPos == std::string::npos || bracePos < semicolonPos)) {
                    searchPos = bracePos + 1;
                    continue;
                }

                if (semicolonPos == std::string::npos) {
                    searchPos++;
                    continue;
                }

                size_t declStart = searchPos;
                size_t declLength = semicolonPos - searchPos + 1;
                std::string declaration = source.substr(declStart, declLength);

                std::string declNoLayout = std::regex_replace(declaration, layoutRegex, "");
                std::string afterUniform = declNoLayout.substr(declNoLayout.find("uniform") + 7);
                trim(afterUniform);
                size_t sep = afterUniform.find_first_of(" \t\r\n[");
                std::string typeToken = (sep == std::string::npos ? afterUniform : afterUniform.substr(0, sep));
                bool isSamplerOrImage =
                        typeToken.rfind("sampler", 0) == 0 ||
                        typeToken.rfind("isampler", 0) == 0 ||
                        typeToken.rfind("usampler", 0) == 0 ||
                        typeToken.rfind("image", 0) == 0;

                if (isSamplerOrImage) {
                    searchPos = declStart + declLength;
                    continue;
                }

                std::string varName = extractVarNameFromUniformDecl(declaration);
                if (allUniformNames.find(varName) == allUniformNames.end()) {
                    mergedUniforms.push_back(declaration);
                    allUniformNames.insert(varName);
                    // save the uniform names in order
                    outUniformBufferNames.emplace_back(varName);
                }

                size_t removeStart = declStart;
                size_t scanBack = declStart;
                while (scanBack > 0 && std::isspace(static_cast<unsigned char>(source[scanBack - 1]))) {
                    scanBack--;
                }
                if (scanBack >= 6) {
                    size_t maybeLayout = source.rfind("layout", scanBack - 6);
                    if (maybeLayout != std::string::npos) {
                        size_t parenOpen = source.find('(', maybeLayout + 6);
                        if (parenOpen != std::string::npos && parenOpen < scanBack) {
                            size_t parenClose = source.find(')', parenOpen);
                            if (parenClose != std::string::npos && parenClose < declStart) {
                                std::string between = source.substr(maybeLayout, declStart - maybeLayout);
                                const std::regex onlyLayout(R"(layout\s*\([^)]*\)\s*)");
                                if (std::regex_match(between, onlyLayout)) {
                                    removeStart = maybeLayout;
                                }
                            }
                        }
                    }
                }
                removalSpans.emplace_back(removeStart, (declStart + declLength) - removeStart);
                searchPos = declStart + declLength;
            }
        }

        if (mergedUniforms.empty()) return;

        for (auto& [id, source] : shaderSources) {
            auto& removalSpans = removalSpansPerShader[id];
            std::sort(removalSpans.rbegin(), removalSpans.rend());
            for (auto& span : removalSpans) {
                source.erase(span.first, span.second);
            }
        }

        std::string uboBlock = "\nlayout(std140) uniform MG_DEFAULT_UBO\n{\n";
        for (auto& decl : mergedUniforms) {
            std::string cleaned = std::regex_replace(decl, layoutRegex, "");
            std::string memberDecl = cleaned.substr(cleaned.find("uniform") + 7);
            size_t semiPos = memberDecl.rfind(';');
            if (semiPos != std::string::npos) {
                memberDecl.erase(semiPos);
            }
            trim(memberDecl);
            uboBlock += "    " + memberDecl + ";\n";
        }
        uboBlock += "};\n";

        MG_Util::Debug::LogD("UBO Generated:\n%s", uboBlock.c_str());

        for (auto& [id, source] : shaderSources) {
            size_t insertPos = 0, lastDirectivePos = 0;
            size_t searchPos = 0;
            while ((searchPos = source.find('#', searchPos)) != std::string::npos) {
                if (isInComment(commentMasks[id], searchPos)) {
                    searchPos++;
                    continue;
                }

                size_t eol = source.find('\n', searchPos);
                if (eol == std::string::npos) break;
                std::string line = source.substr(searchPos, eol - searchPos);
                if (line.find("#version") != std::string::npos ||
                    line.find("#extension") != std::string::npos) {
                    lastDirectivePos = eol + 1;
                }
                searchPos = eol + 1;
            }
            insertPos = lastDirectivePos;
            source.insert(insertPos, uboBlock);
        }
    }
    
    std::string CompileGLSLToTShader(GLenum shaderType, const std::string& source, 
                                     glslang::TShader *&shader, bool isVkGLSL) {
        std::string infoLog;
        using namespace glslang;
        
        int glslVersion = QueryGLSLVersion(source);
        EShLanguage language = GetEShLanguageByShaderType(shaderType);
        if (language == EShLanguage::EShLangCount) {
            infoLog += "Error: [Preprocess] Unsupported shader type: " +
                      std::to_string(shaderType);
            TShader tmpShader(EShLanguage::EShLangVertex);
            return infoLog;
        }

        shader = new TShader(language);
        const char *src = source.c_str();
        shader->setStrings(&src, 1);
        if (isVkGLSL) {
            shader->setPreamble("#define gl_VertexID gl_VertexIndex\n"
                                "#define gl_InstanceID gl_InstanceIndex\n");
        }
        shader->setEnvInput(EShSourceGlsl, language, EShClientVulkan, glslVersion);
        shader->setEnvClient(EShClientOpenGL, isVkGLSL ? EShTargetVulkan_1_3 : EShTargetOpenGL_450);
        shader->setEnvTarget(EShTargetSpv, EShTargetSpv_1_6);
        shader->setAutoMapLocations(true);
        shader->setAutoMapBindings(true);

        // Is InitResources() really correct?
        TBuiltInResource resources = InitResources();

        if (!shader->parse(&resources, glslVersion, true, isVkGLSL ? EShMsgVulkanRules : EShMsgDefault)) {
            infoLog += "Error: [glslang] Cannot compile the " + GetShaderTypeName(shaderType) + ":\n"
                      + std::to_string(shader->getInfoLog());
            return infoLog;
        }
        
        return {};
    }

    std::vector<std::vector<unsigned>> CompileMultipleShadersToSPIRV(const ProgramState& state, ProgramObject& prog, std::string& infoLog) {
        using namespace glslang;

        std::vector<EShLanguage> usedShaderTypes;
        TProgram program;
        for (GLuint shaderId : prog.attachedShaders) {
            auto* shaderObject = state.GetShaderObject(shaderId);
            EShLanguage shLanguage = GetEShLanguageByShaderType(shaderObject->type);
            TShader* shader = nullptr;
            std::string infoLogOfShader = CompileGLSLToTShader(shaderObject->type, shaderObject->source, shader);
            if (!infoLogOfShader.empty()) {
                infoLog = "Error: [glslang] Cannot compile " + GetShaderTypeName(shaderObject->type) +
                        " :\n" + infoLogOfShader;
                return {};
            }
            program.addShader(shader);
            usedShaderTypes.push_back(shLanguage);
        }
        if (!program.link(EShMsgDefault)) {
            infoLog = "Error: [glslang] Cannot link the program:\n" + std::to_string(program.getInfoLog());
            return {};
        }

        std::unique_ptr<glslang::TIoMapResolver> resolver;
        for (unsigned stage = 0; stage < EShLangCount; stage++) {
            auto* pResolver = program.getGlslIoResolver((EShLanguage)stage);
            if (pResolver) {
                resolver = std::unique_ptr<glslang::TIoMapResolver>(pResolver);
                break;
            }
        }
        auto ioMapper = std::unique_ptr<glslang::TIoMapper>(glslang::GetGlslIoMapper());

        if (!program.mapIO(resolver.get(), ioMapper.get())) {
            infoLog = "Error: [glslang] Cannot mapIO:\n" + std::to_string(program.getInfoLog());
            return {};
        }

        SpvOptions spvOptions;
        spvOptions.disableOptimizer = false;

        std::vector<std::vector<unsigned>> allSpirv;
        for(auto type : usedShaderTypes) {
            std::vector<unsigned> spirv;
            GlslangToSpv(*program.getIntermediate(type), spirv, &spvOptions);
            allSpirv.push_back(spirv);
        }

        return allSpirv;
    }
    
    std::vector<unsigned> CompileGLSLToSPIRV(GLenum shaderType, const std::string &source, 
                                             std::string &infoLog, bool isVkGLSL, GLenum additionalShaderType,
                                             const std::string& additionalShaderSource
                                             ) {
        using namespace glslang;

        TShader* shader = nullptr;
        std::string infoLogOfShader = CompileGLSLToTShader(shaderType, source, shader, isVkGLSL);
        if (!infoLogOfShader.empty()) {
            infoLog = "Error: [glslang] Cannot compile " + GetShaderTypeName(shaderType) + ":\n" + infoLogOfShader;
            return {};
        }
        TProgram program;
        
        if (additionalShaderType == GL_VERTEX_SHADER)
            program.addShader(shader);
        if (additionalShaderType && !additionalShaderSource.empty()) {
            TShader* additionalShader = nullptr;
            std::string additionalInfoLog = CompileGLSLToTShader(additionalShaderType, additionalShaderSource, additionalShader, isVkGLSL);
            if (!additionalInfoLog.empty()) {
                infoLog = "Error: [glslang] Cannot compile additional " + GetShaderTypeName(additionalShaderType) + ":\n" + additionalInfoLog;
                return {};
            }
            program.addShader(additionalShader);
        }
        if (additionalShaderType != GL_VERTEX_SHADER)
            program.addShader(shader);
        
        if (!program.link(isVkGLSL ? EShMsgVulkanRules : EShMsgDefault)) {
            infoLog = "Error: [glslang] Cannot link the program of the single shader:\n" + std::to_string(program.getInfoLog());
            return {};
        }
        program.mapIO();
        
        std::vector<unsigned> spirv;
        
        SpvOptions spvOptions;
        spvOptions.disableOptimizer = false;
        EShLanguage language = GetEShLanguageByShaderType(shaderType);
        GlslangToSpv(*program.getIntermediate(language), spirv, &spvOptions);

        return spirv;
    }

    void ReflectSPIRVUniforms(const std::vector<std::vector<unsigned>>& allSpirv, ProgramObject& prog, std::string& infoLog) {
        spvc_context context = nullptr;

        if (spvc_context_create(&context) != SPVC_SUCCESS) {
            infoLog = "Failed to create SPIRV-Cross context";
            return;
        }

        MG_Global::unordered_set<GLint> used_locations;
        GLint auto_location = 0;
        for (auto spirv: allSpirv) {
            spvc_parsed_ir ir = nullptr;
            spvc_compiler compiler = nullptr;
            spvc_resources resources = nullptr;
            
            spvc_result result = spvc_context_parse_spirv(context, spirv.data(), spirv.size(), &ir);
            if (result != SPVC_SUCCESS) {
                infoLog = "SPIR-V parsing failed";
                spvc_context_destroy(context);
                return;
            }

            result = spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir,
                                                  SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);
            if (result != SPVC_SUCCESS) {
                infoLog = "Failed to create SPIRV-Cross compiler";
                spvc_context_destroy(context);
                return;
            }

            result = spvc_compiler_create_shader_resources(compiler, &resources);
            if (result != SPVC_SUCCESS) {
                infoLog = "Failed to get shader resources";
                spvc_context_destroy(context);
                return;
            }

            const spvc_reflected_resource *uniform_buffers = nullptr;
            const spvc_reflected_resource *uniform_vars = nullptr;
            const spvc_reflected_resource *sampled_images = nullptr;
            size_t uniform_buffer_count = 0;
            size_t uniform_var_count = 0;
            size_t sampled_image_count = 0;

            spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER,
                                                      &uniform_buffers, &uniform_buffer_count);
            spvc_resources_get_resource_list_for_type(resources,
                                                      SPVC_RESOURCE_TYPE_GL_PLAIN_UNIFORM,
                                                      &uniform_vars, &uniform_var_count);
            spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE,
                                                      &sampled_images, &sampled_image_count);

            std::vector<const spvc_reflected_resource *> all_resources;
            for (size_t i = 0; i < uniform_buffer_count; ++i)
                all_resources.push_back(&uniform_buffers[i]);
            for (size_t i = 0; i < uniform_var_count; ++i)
                all_resources.push_back(&uniform_vars[i]);
            for (size_t i = 0; i < sampled_image_count; ++i)
                all_resources.push_back(&sampled_images[i]);

            for (const auto res_ptr: all_resources) {
                const spvc_reflected_resource &res = *res_ptr;
                const char *name = res.name;
                if (name[0] == '_') continue;
                
                unsigned spirv_location = spvc_compiler_get_decoration(compiler, res.id,
                                                                       SpvDecorationLocation);
                GLint final_location;

                spvc_type_id type_id = res.type_id;
                spvc_type type = spvc_compiler_get_type_handle(compiler, type_id);
                spvc_basetype base_type = spvc_type_get_basetype(type);
                GLenum gl_type = GL_NONE;

                if (spirv_location >= 0 && base_type != SPVC_BASETYPE_SAMPLED_IMAGE) {
                    auto conflict_it = used_locations.find(spirv_location);
                    if (conflict_it != used_locations.end()) {
                        // Check if the conflicting uniform is a sampler
                        std::string conflicting_name;
                        for (const auto& entry : prog.uniformLocations) {
                            if (entry.second == spirv_location) {
                                conflicting_name = entry.first;
                                break;
                            }
                        }
                        if(conflicting_name == std::string(name)) {
                            continue;
                        } else if (!conflicting_name.empty() && prog.uniformValues.count(conflicting_name) &&
                                MG_GL::GL::IsSamplerType(prog.uniformValues[conflicting_name].type)) {
                            // Reassign location for the sampler
                            GLint new_sampler_loc = auto_location++;
                            while (used_locations.find(new_sampler_loc) != used_locations.end()) new_sampler_loc++;
                            prog.uniformLocations[conflicting_name] = new_sampler_loc;
                            used_locations.erase(conflict_it); // Remove old sampler location
                            used_locations.insert(new_sampler_loc); // Add new sampler location
                        } else {
                            infoLog += "\nLocation conflict for uniform: " + std::string(name) + " at location " + std::to_string(spirv_location);
                            infoLog += "\nExisting uniforms and locations:";
                            for (const auto& entry : prog.uniformLocations) {
                                infoLog += "\n  " + entry.first + ": " + std::to_string(entry.second);
                            }
                             infoLog += "\nUsed locations set:";
                            for (GLint loc : used_locations) infoLog += " " + std::to_string(loc);
                            continue;
                        }
                    }
                    final_location = spirv_location;
                } else {
                    while (used_locations.find(auto_location) != used_locations.end()) {
                        auto_location++;
                    }
                    final_location = auto_location;
                }

                used_locations.insert(final_location);

                if (base_type == SPVC_BASETYPE_SAMPLED_IMAGE) {
                    SpvDim dim = spvc_type_get_image_dimension(type);
                    bool is_array = spvc_type_get_image_arrayed(type);
                    bool is_shadow = spvc_type_get_image_is_depth(type);
                    bool is_ms = spvc_type_get_image_multisampled(type);

                    switch (dim) {
                        case SpvDim1D:
                            gl_type = is_array ? GL_SAMPLER_1D_ARRAY : GL_SAMPLER_1D;
                            if (is_shadow)
                                gl_type = is_array ? GL_SAMPLER_1D_ARRAY_SHADOW
                                                   : GL_SAMPLER_1D_SHADOW;
                            break;
                        case SpvDim2D:
                            if (is_ms) {
                                gl_type = is_array ? GL_SAMPLER_2D_MULTISAMPLE_ARRAY
                                                   : GL_SAMPLER_2D_MULTISAMPLE;
                            } else {
                                gl_type = is_array ? GL_SAMPLER_2D_ARRAY : GL_SAMPLER_2D;
                                if (is_shadow)
                                    gl_type = is_array ? GL_SAMPLER_2D_ARRAY_SHADOW
                                                       : GL_SAMPLER_2D_SHADOW;
                            }
                            break;
                        case SpvDim3D:
                            gl_type = GL_SAMPLER_3D;
                            break;
                        case SpvDimCube:
                            gl_type = is_array ? GL_SAMPLER_CUBE_MAP_ARRAY : GL_SAMPLER_CUBE;
                            if (is_shadow)
                                gl_type = is_array ? GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW
                                                   : GL_SAMPLER_CUBE_SHADOW;
                            break;
                        case SpvDimRect:
                            gl_type = is_shadow ? GL_SAMPLER_2D_RECT_SHADOW : GL_SAMPLER_2D_RECT;
                            break;
                        case SpvDimBuffer:
                            gl_type = GL_SAMPLER_BUFFER;
                            break;
                        default:
                            gl_type = GL_SAMPLER_2D;
                    }

                    spvc_basetype sampled_base = spvc_type_get_basetype(type);
                    if (sampled_base == SPVC_BASETYPE_INT32) {
                        switch (gl_type) {
                            case GL_SAMPLER_1D:
                                gl_type = GL_INT_SAMPLER_1D;
                                break;
                            case GL_SAMPLER_2D:
                                gl_type = GL_INT_SAMPLER_2D;
                                break;
                            case GL_SAMPLER_3D:
                                gl_type = GL_INT_SAMPLER_3D;
                                break;
                            case GL_SAMPLER_CUBE:
                                gl_type = GL_INT_SAMPLER_CUBE;
                                break;
                            case GL_SAMPLER_1D_ARRAY:
                                gl_type = GL_INT_SAMPLER_1D_ARRAY;
                                break;
                            case GL_SAMPLER_2D_ARRAY:
                                gl_type = GL_INT_SAMPLER_2D_ARRAY;
                                break;
                            case GL_SAMPLER_CUBE_MAP_ARRAY:
                                gl_type = GL_INT_SAMPLER_CUBE_MAP_ARRAY;
                                break;
                            case GL_SAMPLER_2D_RECT:
                                gl_type = GL_INT_SAMPLER_2D_RECT;
                                break;
                            case GL_SAMPLER_BUFFER:
                                gl_type = GL_INT_SAMPLER_BUFFER;
                                break;
                            case GL_SAMPLER_2D_MULTISAMPLE:
                                gl_type = GL_INT_SAMPLER_2D_MULTISAMPLE;
                                break;
                            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                                gl_type = GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
                                break;
                        }
                    } else if (sampled_base == SPVC_BASETYPE_UINT32) {
                        switch (gl_type) {
                            case GL_SAMPLER_1D:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_1D;
                                break;
                            case GL_SAMPLER_2D:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_2D;
                                break;
                            case GL_SAMPLER_3D:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_3D;
                                break;
                            case GL_SAMPLER_CUBE:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_CUBE;
                                break;
                            case GL_SAMPLER_1D_ARRAY:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_1D_ARRAY;
                                break;
                            case GL_SAMPLER_2D_ARRAY:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
                                break;
                            case GL_SAMPLER_CUBE_MAP_ARRAY:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY;
                                break;
                            case GL_SAMPLER_2D_RECT:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_2D_RECT;
                                break;
                            case GL_SAMPLER_BUFFER:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_BUFFER;
                                break;
                            case GL_SAMPLER_2D_MULTISAMPLE:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
                                break;
                            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                                gl_type = GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
                                break;
                        }
                    }
                } else {
                    unsigned vec_size = spvc_type_get_vector_size(type);
                    unsigned columns = spvc_type_get_columns(type);

                    switch (base_type) {
                        case SPVC_BASETYPE_FP32:
                            if (columns > 1) {
                                if (vec_size == 2 && columns == 2) gl_type = GL_FLOAT_MAT2;
                                else if (vec_size == 3 && columns == 3) gl_type = GL_FLOAT_MAT3;
                                else if (vec_size == 4 && columns == 4) gl_type = GL_FLOAT_MAT4;
                                else if (vec_size == 3 && columns == 2) gl_type = GL_FLOAT_MAT2x3;
                                else if (vec_size == 4 && columns == 2) gl_type = GL_FLOAT_MAT2x4;
                                else if (vec_size == 2 && columns == 3) gl_type = GL_FLOAT_MAT3x2;
                                else if (vec_size == 4 && columns == 3) gl_type = GL_FLOAT_MAT3x4;
                                else if (vec_size == 2 && columns == 4) gl_type = GL_FLOAT_MAT4x2;
                                else if (vec_size == 3 && columns == 4) gl_type = GL_FLOAT_MAT4x3;
                            } else {
                                switch (vec_size) {
                                    case 1:
                                        gl_type = GL_FLOAT;
                                        break;
                                    case 2:
                                        gl_type = GL_FLOAT_VEC2;
                                        break;
                                    case 3:
                                        gl_type = GL_FLOAT_VEC3;
                                        break;
                                    case 4:
                                        gl_type = GL_FLOAT_VEC4;
                                        break;
                                }
                            }
                            break;
                        case SPVC_BASETYPE_INT32:
                            switch (vec_size) {
                                case 1:
                                    gl_type = GL_INT;
                                    break;
                                case 2:
                                    gl_type = GL_INT_VEC2;
                                    break;
                                case 3:
                                    gl_type = GL_INT_VEC3;
                                    break;
                                case 4:
                                    gl_type = GL_INT_VEC4;
                                    break;
                            }
                            break;
                        case SPVC_BASETYPE_UINT32:
                            switch (vec_size) {
                                case 1:
                                    gl_type = GL_UNSIGNED_INT;
                                    break;
                                case 2:
                                    gl_type = GL_UNSIGNED_INT_VEC2;
                                    break;
                                case 3:
                                    gl_type = GL_UNSIGNED_INT_VEC3;
                                    break;
                                case 4:
                                    gl_type = GL_UNSIGNED_INT_VEC4;
                                    break;
                            }
                            break;
                        case SPVC_BASETYPE_BOOLEAN:
                            switch (vec_size) {
                                case 1:
                                    gl_type = GL_BOOL;
                                    break;
                                case 2:
                                    gl_type = GL_BOOL_VEC2;
                                    break;
                                case 3:
                                    gl_type = GL_BOOL_VEC3;
                                    break;
                                case 4:
                                    gl_type = GL_BOOL_VEC4;
                                    break;
                            }
                            break;
                        default:
                            continue;
                    }
                }

                prog.uniformLocations[name] = final_location;
                UniformValue uniformValue;
                uniformValue.type = gl_type;
                prog.uniformValues[name] = uniformValue;
            }
        }
        spvc_context_destroy(context);
    }

    std::string CompileSPIRVToGLSL(std::vector<unsigned int> spirv, uint glslVersion, bool isES, bool isVkGLSL) {
        spvc_context context = nullptr;
        spvc_parsed_ir ir = nullptr;
        spvc_compiler compiler_glsl = nullptr;
        spvc_compiler_options options = nullptr;
        spvc_resources resources = nullptr;
        const spvc_reflected_resource *list = nullptr;
        const char *result = nullptr;
        size_t count;

        const SpvId *p_spirv = spirv.data();
        size_t word_count = spirv.size();

        spvc_context_create(&context);
        spvc_context_parse_spirv(context, p_spirv, word_count, &ir);
        spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);
        spvc_compiler_create_shader_resources(compiler_glsl, &resources);
        spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
        spvc_compiler_create_compiler_options(compiler_glsl, &options);
        
        spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, glslVersion);
        spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, isES ? SPVC_TRUE : SPVC_FALSE);
        spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_VULKAN_SEMANTICS, isVkGLSL ? SPVC_TRUE : SPVC_FALSE);
        spvc_compiler_install_compiler_options(compiler_glsl, options);
        spvc_compiler_compile(compiler_glsl, &result);

        if (!result) {
            MG_Util::Debug::LogE("Failed to compile the shader to GLSL: %s", spvc_context_get_last_error_string(context));
            return{};
        }

        std::string essl = result;

        spvc_context_destroy(context);

        return essl;
    }
}
