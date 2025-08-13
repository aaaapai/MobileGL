//
// Created by Swung 0x48 on 2025/7/17.
//
#include <gtest/gtest.h>

#include "Includes.h"
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/ShaderTranspiler/ShaderCompiler.h>
#include <MG_Util/ShaderTranspiler/Types.h>

#include "MG_Util/ShaderTranspiler/glslang/UniformTraverser.h"
using namespace MobileGL;

class ProgramUtilTest : public ::testing::Test {
protected:
};

TEST_F(ProgramUtilTest, Sanity) {
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

TEST_F(ProgramUtilTest, CompileSimpleVertexShader) {
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

TEST_F(ProgramUtilTest, CompileSimpleFragmentShader) {
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

const char* position_color_fsh = R"(#version 150

in vec4 vertexColor;

uniform vec4 ColorModulator;

out vec4 fragColor;

void main() {
    vec4 color = vertexColor;
    if (color.a == 0.0) {
        discard;
    }
    fragColor = color * ColorModulator;
})";

TEST_F(ProgramUtilTest, CompileFragmentShaderWithDiscard) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
        .shaderType = GL_FRAGMENT_SHADER,
        .sourceStr = position_color_fsh
};
    auto res = ShaderCompiler::CompileShader(attrib);
    if (!res) {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }

    ProgramAttrib programAttrib {
        .shaderTypes = { GL_FRAGMENT_SHADER },
        .shaders = { res.value() }
    };

    auto program_res = ShaderCompiler::LinkProgram(programAttrib);
    if (!program_res) {
        ASSERT_NE(program_res.error().errc, 0);
        FAIL() << "errc: " << program_res.error().errc << "\nlog: " << program_res.error().log;
    }


    auto spirvs = program_res.value();

    Vector<SpvcSession> sessions(spirvs.size());
    for (SizeT i = 0; i < spirvs.size(); ++i) {
        sessions[i] = SpvcSession(spirvs[i]);
    }

    for (SizeT i = 0; i < spirvs.size(); ++i) {
        std::cout << "Decompiling " << MG_Util::ConvertGLEnumToString(programAttrib.shaderTypes[i]) << std::endl;
        auto src = ShaderCompiler::DecompileShader(sessions[i]);
        if (!src) {
            ASSERT_NE(src.error().errc, 0);
            FAIL() << "errc: " << src.error().errc << "\nlog: " << src.error().log;
        } else {
            std::cout << src.value() << std::endl;
        }

        if (src.value().find("demote") != std::string::npos) {
            FAIL() << "Found unsupported demote!";
        }
    }
}

const char* vs_location = R"(#version 460

in vec4 Position;

layout(location = 1) uniform mat4 ProjMat;
layout(location = 20) uniform vec2 InSize;
uniform vec2 OutSize;

out vec2 texCoord;
out vec2 oneTexel;

void main(){
    vec4 outPos = ProjMat * vec4(Position.xy, 0.0, 1.0);
    gl_Position = vec4(outPos.xy, 0.2, 1.0);

    oneTexel = 1.0 / InSize;

    texCoord = Position.xy / OutSize;
})";

TEST_F(ProgramUtilTest, CompileVertexShaderWithLocation) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
        .shaderType = GL_VERTEX_SHADER,
        .sourceStr = vs_location,
        .flags = ShaderCompileBits::CompileForOpenGL
    };
    auto res = ShaderCompiler::CompileShader(attrib);
    if (!res) {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }
    UnorderedMap<String, Int> uniforms;

    auto pShader = res.value();
    auto root = pShader->getIntermediate()->getTreeRoot();
    UniformTraverser traverser;
    root->traverse(&traverser);
    auto& symbols = traverser.GetCollectedSymbols();
    for (const auto& symbol : symbols) {
        uniforms[symbol->getName().c_str()] = symbol->getQualifier().layoutLocation;
    }

    EXPECT_EQ(uniforms["ProjMat"], 1);
    EXPECT_EQ(uniforms["InSize"], 20);
    EXPECT_EQ(uniforms["OutSize"], 4095);
}

TEST_F(ProgramUtilTest, CompileAndLinkProgram) {
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

TEST_F(ProgramUtilTest, DecompProgram) {
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

    Vector<SpvcSession> sessions(spirvs.size());
    for (SizeT i = 0; i < spirvs.size(); ++i) {
        sessions[i] = SpvcSession(spirvs[i]);
    }

    for (SizeT i = 0; i < spirvs.size(); ++i) {
        std::cout << "Decompiling " << MG_Util::ConvertGLEnumToString(programAttrib.shaderTypes[i]) << std::endl;
        auto src = ShaderCompiler::DecompileShader(sessions[i]);
        if (!src) {
            ASSERT_NE(src.error().errc, 0);
            FAIL() << "errc: " << src.error().errc << "\nlog: " << src.error().log;
        } else {
            std::cout << src.value() << std::endl;
        }
    }

    // spirv link check
    auto vs_outputs = sessions[0].GetShaderInterface(SPVC_RESOURCE_TYPE_STAGE_OUTPUT);
    auto fs_inputs = sessions[1].GetShaderInterface(SPVC_RESOURCE_TYPE_STAGE_INPUT);

    ASSERT_EQ(vs_outputs.size(), fs_inputs.size());

    for (size_t i = 0; i < vs_outputs.size(); ++i) {
        EXPECT_EQ(vs_outputs[i].location, fs_inputs[i].location);
    }
    
    auto vs_uniforms = sessions[0].GetShaderInterface(SPVC_RESOURCE_TYPE_GL_PLAIN_UNIFORM);
    auto fs_uniforms = sessions[1].GetShaderInterface(SPVC_RESOURCE_TYPE_GL_PLAIN_UNIFORM);

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

    auto vs_samplers = sessions[0].GetShaderInterface(SPVC_RESOURCE_TYPE_SAMPLED_IMAGE);
    auto fs_samplers = sessions[1].GetShaderInterface(SPVC_RESOURCE_TYPE_SAMPLED_IMAGE);

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

    auto& meta0 = sessions[0].GetMetadata();
    auto& meta1 = sessions[1].GetMetadata();

    for (auto& [name, offset] : meta0.plainUniformOffsetsInUBO) {
        printf("%s: \t%u\n", name.c_str(), offset);
    }

    printf("\n");

    for (auto& [name, offset] : meta1.plainUniformOffsetsInUBO) {
        printf("%s: \t%u\n", name.c_str(), offset);
    }

    EXPECT_EQ(meta0.plainUniformOffsetsInUBO.size(), meta1.plainUniformOffsetsInUBO.size());
    for (auto& [name, offset] : meta0.plainUniformOffsetsInUBO) {
        EXPECT_EQ(offset, meta1.plainUniformOffsetsInUBO.at(name));
    }
}
