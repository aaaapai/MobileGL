//
// Created by Swung 0x48 on 2025/7/17.
//
#include <gtest/gtest.h>

#include "Includes.h"

using namespace MobileGL;

class ProgramTest : public ::testing::Test {
protected:
};

TEST_F(ProgramTest, Sanity) {
    ASSERT_TRUE(true);
}

const char* vs = R"(#version 150

in vec3 Position;
in vec2 UV;
in vec4 Color;

uniform mat4 ModelViewMat;
uniform mat4 ProjMat;

out vec2 texCoord;
out vec4 vertexColor;

void main() {
    gl_Position = ProjMat * ModelViewMat * vec4(Position, 1.0);

    texCoord = UV;
    vertexColor = Color;
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

uniform sampler2D DiffuseSampler;

uniform vec4 ColorModulator;

in vec2 texCoord;
in vec4 vertexColor;

out vec4 fragColor;

void main() {
    vec4 color = texture(DiffuseSampler, texCoord) * vertexColor;

    // blit final output of compositor into displayed back buffer
    fragColor = color * ColorModulator;
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

const char* vs_uniform = R"(#version 150

in vec3 Position;
in vec2 UV;
in vec4 Color;

uniform mat4 ModelViewMat;
uniform mat4 ProjMat;

out vec2 texCoord;
out vec4 vertexColor;

void main() {
    mat4 mat = ProjMat * ModelViewMat;
    gl_Position = ProjMat * ModelViewMat * vec4(Position, 1.0);

    texCoord = UV;
    vertexColor = Color;
})";



TEST_F(ProgramTest, ExtractPlainUniform) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
        .shaderType = GL_VERTEX_SHADER,
        .sourceStr = vs_uniform
    };

    Vector<TUniform<TUniformType::Uniform>> uniforms;
    Vector<TUniform<TUniformType::Sampler>> samplers;

    auto res = ShaderCompiler::CompileShader(attrib);
    if (res) {
        UniformTraverser traverser(uniforms, samplers);
        auto root = res->TShader->getIntermediate()->getTreeRoot();
        root->traverse(&traverser);

        ASSERT_EQ(uniforms.size(), 2);
        ASSERT_EQ(samplers.size(), 0);

        auto ProjMat_uniform_it = std::find_if(uniforms.begin(), uniforms.end(), [] (TUniform<TUniformType::Uniform>& uniform) {
           return uniform.name == "ProjMat";
        });
        ASSERT_TRUE(ProjMat_uniform_it != uniforms.end());
        ASSERT_EQ(ProjMat_uniform_it->storageQualifier, glslang::EvqUniform);

        auto ModelViewMat_uniform_it = std::find_if(uniforms.begin(), uniforms.end(), [] (TUniform<TUniformType::Uniform>& uniform) {
           return uniform.name == "ModelViewMat";
        });
        ASSERT_TRUE(ModelViewMat_uniform_it != uniforms.end());
        ASSERT_EQ(ModelViewMat_uniform_it->storageQualifier, glslang::EvqUniform);
    } else {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }
}

const char* fs_uniform = R"(#version 150

uniform sampler2D DiffuseSampler;

uniform vec4 ColorModulator;

in vec2 texCoord;
in vec4 vertexColor;

out vec4 fragColor;

void main() {
    vec4 color = texture(DiffuseSampler, texCoord) * vertexColor;

    // blit final output of compositor into displayed back buffer
    fragColor = color * ColorModulator;
})";

TEST_F(ProgramTest, ExtractSampler) {
    using namespace MG_Util::ShaderTranspiler;
    ShaderAttrib attrib {
        .shaderType = GL_FRAGMENT_SHADER,
        .sourceStr = fs_uniform
    };

    Vector<TUniform<TUniformType::Uniform>> uniforms;
    Vector<TUniform<TUniformType::Sampler>> samplers;

    auto res = ShaderCompiler::CompileShader(attrib);
    if (res) {
        UniformTraverser traverser(uniforms, samplers);
        auto root = res->TShader->getIntermediate()->getTreeRoot();
        root->traverse(&traverser);

        ASSERT_EQ(uniforms.size(), 1);
        ASSERT_EQ(samplers.size(), 1);

        auto ColorModulator_uniform_it = std::find_if(uniforms.begin(), uniforms.end(), [] (TUniform<TUniformType::Uniform>& uniform) {
           return uniform.name == "ColorModulator";
        });
        ASSERT_TRUE(ColorModulator_uniform_it != uniforms.end());
        ASSERT_EQ(ColorModulator_uniform_it->storageQualifier, glslang::EvqUniform);

        auto DiffuseSampler_uniform_it = std::find_if(samplers.begin(), samplers.end(), [] (TUniform<TUniformType::Sampler>& uniform) {
           return uniform.name == "DiffuseSampler";
        });
        ASSERT_TRUE(DiffuseSampler_uniform_it != samplers.end());
        auto& sampler = DiffuseSampler_uniform_it->sampler;
        ASSERT_EQ(sampler.type, glslang::EbtFloat);
        ASSERT_EQ(sampler.dim, glslang::Esd2D);
    } else {
        ASSERT_NE(res.error().errc, 0);
        FAIL() << "errc: " << res.error().errc << "\nlog: " << res.error().log;
    }
}