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
