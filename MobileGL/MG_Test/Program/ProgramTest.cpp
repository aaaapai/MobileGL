#include <gtest/gtest.h>
#include "Includes.h"
#include "MG_Impl/GLImpl/Program/GL_Program.h"
#include "MG_State/GLState/Core.h"

using namespace MobileGL;
using namespace MobileGL::MG_Impl::GLImpl;

class ProgramTest : public ::testing::Test {
protected:
    ProgramTest() {
        mGLContext = MakeUnique<MG_State::GLState::GLContext>();
        MG_State::pGLContext = mGLContext.get();
    }
private:
    UniquePtr<MG_State::GLState::GLContext> mGLContext;
};

TEST_F(ProgramTest, Sanity) {
    ASSERT_TRUE(true);
}

const char* vsSrc = R"(#version 460

in vec4 Position;

layout(location = 0) uniform mat4 ProjMat;
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

const char* fsSrc = R"(#version 460

uniform sampler2D InSampler;

in vec2 texCoord;
in vec2 oneTexel;

uniform vec2 InSize;

layout(location = 1) uniform vec3 Gray;
uniform vec3 RedMatrix;
uniform vec3 GreenMatrix;
uniform vec3 BlueMatrix;
uniform vec3 Offset;
uniform vec3 ColorScale;
layout(location = 6) uniform float Saturation;

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


TEST_F(ProgramTest, CompileVertex) {
    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    CompileShader(vs);
}

TEST_F(ProgramTest, CompileFragment) {
    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    CompileShader(fs);
}

TEST_F(ProgramTest, CompileAndLink) {
    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    printf("Compiling vertex shader: %s\n", vsSrc);
    CompileShader(vs);
    printf("Compiled vertex shader.\n");

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    printf("Compiling fragment shader: %s\n", fsSrc);
    CompileShader(fs);
    printf("Compiled fragment shader.\n");

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);
    printf("Linking program...\n");
    LinkProgram(program);
    printf("Program linked...\n");

    EXPECT_EQ(GetUniformLocation(program, "ProjMat"), 0);
    EXPECT_EQ(GetUniformLocation(program, "Gray"), 1);
    EXPECT_EQ(GetUniformLocation(program, "Saturation"), 6);
}
