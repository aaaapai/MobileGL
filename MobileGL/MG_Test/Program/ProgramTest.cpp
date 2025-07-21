//
// Created by Swung 0x48 on 2025/7/17.
//
#include <gtest/gtest.h>

#include "Includes.h"
using namespace MobileGL;

class ProgramTest : public ::testing::Test {
protected:
};

struct InterfaceVariable {
    std::string name;
    uint32_t location;

    bool operator<(const InterfaceVariable& other) const {
        return location < other.location;
    }

    bool operator==(const InterfaceVariable& other) const {
        return location == other.location && name == other.name;
    }
};

static std::vector<InterfaceVariable> GetShaderInterface(const std::vector<unsigned int>& spirv, spvc_resource_type resource_type) {
    spvc_context context = nullptr;
    spvc_context_create(&context);

    spvc_parsed_ir ir = nullptr;
    spvc_context_parse_spirv(context, spirv.data(), spirv.size(), &ir);

    spvc_compiler compiler = nullptr;
    spvc_context_create_compiler(context, SPVC_BACKEND_NONE, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler);

    spvc_resources resources = nullptr;
    spvc_compiler_create_shader_resources(compiler, &resources);

    const spvc_reflected_resource *list = nullptr;
    size_t count = 0;
    spvc_resources_get_resource_list_for_type(resources, resource_type, &list, &count);

    std::vector<InterfaceVariable> variables;
    for (size_t i = 0; i < count; ++i) {
        unsigned int builtin;
        if (spvc_compiler_has_decoration(compiler, list[i].id, SpvDecorationBuiltIn)) {
            continue;
        }

        InterfaceVariable var;
        var.name = list[i].name;
        var.location = spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationLocation);
        variables.push_back(var);
    }

    spvc_context_destroy(context);

    std::sort(variables.begin(), variables.end());

    return variables;
}

TEST_F(ProgramTest, Sanity) {
    ASSERT_TRUE(true);
}

const char* vs = R"(#version 150

in vec4 Position;

uniform mat4 ProjMat;
uniform vec2 InSize;
uniform vec2 OutSize;

out vec2 texCoord;
out vec2 oneTexel;

void main(){
    vec4 outPos = ProjMat * vec4(Position.xy, 0.0, 1.0);
    gl_Position = vec4(outPos.xy, 0.2, 1.0);

    oneTexel = 1.0 / InSize;

    texCoord = Position.xy / OutSize;
})";

TEST_F(ProgramTest, CompileSimpleVertexShader) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
        .shaderType = GL_VERTEX_SHADER,
        .sourceStr = vs
    };
    auto res = ShaderCompiler::CompileShader(attrib);
    if (!res) {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }
}

const char* fs = R"(#version 150

uniform sampler2D InSampler;

in vec2 texCoord;
in vec2 oneTexel;

uniform vec2 InSize;

uniform vec3 Gray;
uniform vec3 RedMatrix;
uniform vec3 GreenMatrix;
uniform vec3 BlueMatrix;
uniform vec3 Offset;
uniform vec3 ColorScale;
uniform float Saturation;

out vec4 fragColor;

void main() {
    vec4 InTexel = texture(InSampler, texCoord);

    // Color Matrix
    float RedValue = dot(InTexel.rgb, RedMatrix);
    float GreenValue = dot(InTexel.rgb, GreenMatrix);
    float BlueValue = dot(InTexel.rgb, BlueMatrix);
    vec3 OutColor = vec3(RedValue, GreenValue, BlueValue);

    // Offset & Scale
    OutColor = (OutColor * ColorScale) + Offset;

    // Saturation
    float Luma = dot(OutColor, Gray);
    vec3 Chroma = OutColor - Luma;
    OutColor = (Chroma * Saturation) + Luma;

    fragColor = vec4(OutColor, 1.0);
})";

TEST_F(ProgramTest, CompileSimpleFragmentShader) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
            .shaderType = GL_FRAGMENT_SHADER,
            .sourceStr = fs
    };
    auto res = ShaderCompiler::CompileShader(attrib);
    if (!res) {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }
}

TEST_F(ProgramTest, CompileAndLinkProgram) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib vs_attrib {
        .shaderType = GL_VERTEX_SHADER,
        .sourceStr = vs
    };
    auto vs_res = ShaderCompiler::CompileShader(vs_attrib);
    if (!vs_res) {
        ASSERT_NE(vs_res.error().errc, 0);
        FAIL() << "errc: " << vs_res.error().errc << "\nlog: " << vs_res.error().log;
    }

    ShaderAttrib fs_attrib {
        .shaderType = GL_FRAGMENT_SHADER,
        .sourceStr = fs
};
    auto fs_res = ShaderCompiler::CompileShader(fs_attrib);
    if (!fs_res) {
        ASSERT_NE(fs_res.error().errc, 0);
        FAIL() << "errc: " << fs_res.error().errc << "\nlog: " << fs_res.error().log;
    }

    ProgramAttrib programAttrib {
        .shaderTypes = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
        .shaders = { vs_res.value(), fs_res.value() }
    };

    auto program_res = ShaderCompiler::LinkProgram(programAttrib);
    if (!program_res) {
        ASSERT_NE(program_res.error().errc, 0);
        FAIL() << "errc: " << program_res.error().errc << "\nlog: " << program_res.error().log;
    }
}

TEST_F(ProgramTest, DecompProgram) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib vs_attrib {
        .shaderType = GL_VERTEX_SHADER,
        .sourceStr = vs
    };
    auto vs_res = ShaderCompiler::CompileShader(vs_attrib);
    if (!vs_res) {
        ASSERT_NE(vs_res.error().errc, 0);
        FAIL() << "errc: " << vs_res.error().errc << "\nlog: " << vs_res.error().log;
    }

    ShaderAttrib fs_attrib {
        .shaderType = GL_FRAGMENT_SHADER,
        .sourceStr = fs
};
    auto fs_res = ShaderCompiler::CompileShader(fs_attrib);
    if (!fs_res) {
        ASSERT_NE(fs_res.error().errc, 0);
        FAIL() << "errc: " << fs_res.error().errc << "\nlog: " << fs_res.error().log;
    }

    ProgramAttrib programAttrib {
        .shaderTypes = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER },
        .shaders = { vs_res.value(), fs_res.value() }
    };

    auto program_res = ShaderCompiler::LinkProgram(programAttrib);
    if (!program_res) {
        ASSERT_NE(program_res.error().errc, 0);
        FAIL() << "errc: " << program_res.error().errc << "\nlog: " << program_res.error().log;
    }

    auto spirvs = program_res.value();

    for (SizeT i = 0; i < spirvs.size(); ++i) {
        std::cout << "Decompiling " << MG_Util::ConvertGLEnumToString(programAttrib.shaderTypes[i]) << std::endl;
        auto src = ShaderCompiler::DecompileShader(spirvs[i]);
        if (!src) {
            ASSERT_NE(src.error().errc, 0);
            FAIL() << "errc: " << src.error().errc << "\nlog: " << src.error().log;
        } else {
            std::cout << src.value() << std::endl;
        }
    }

    // spirv link check
    auto vs_outputs = GetShaderInterface(spirvs[0], SPVC_RESOURCE_TYPE_STAGE_OUTPUT);
    auto fs_inputs = GetShaderInterface(spirvs[1], SPVC_RESOURCE_TYPE_STAGE_INPUT);

    ASSERT_EQ(vs_outputs.size(), fs_inputs.size());

    for (size_t i = 0; i < vs_outputs.size(); ++i) {
        EXPECT_EQ(vs_outputs[i].location, fs_inputs[i].location);
    }
    
    auto vs_uniforms = GetShaderInterface(spirvs[0], SPVC_RESOURCE_TYPE_GL_PLAIN_UNIFORM);
    auto fs_uniforms = GetShaderInterface(spirvs[1], SPVC_RESOURCE_TYPE_GL_PLAIN_UNIFORM);

    std::unordered_map<std::string, uint32_t> uniform_locations;
    for (const auto& uniform : vs_uniforms) {
        uniform_locations[uniform.name] = uniform.location;
    }

    for (const auto& uniform : fs_uniforms) {
        auto it = uniform_locations.find(uniform.name);
        if (it != uniform_locations.end()) {
            EXPECT_EQ(it->second, uniform.location);
        }
    }

    auto vs_samplers = GetShaderInterface(spirvs[0], SPVC_RESOURCE_TYPE_SAMPLED_IMAGE);
    auto fs_samplers = GetShaderInterface(spirvs[1], SPVC_RESOURCE_TYPE_SAMPLED_IMAGE);

    std::unordered_map<std::string, uint32_t> sampler_locations;
    for (const auto& uniform : vs_uniforms) {
        sampler_locations[uniform.name] = uniform.location;
    }

    for (const auto& uniform : fs_uniforms) {
        auto it = sampler_locations.find(uniform.name);
        if (it != sampler_locations.end()) {
            EXPECT_EQ(it->second, uniform.location);
        }
    }
}
