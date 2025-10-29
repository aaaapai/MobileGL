#include <gtest/gtest.h>
#include "Includes.h"
#include "MG_Impl/GLImpl/Program/GL_Program.h"
#include "MG_State/GLState/Core.h"

using namespace MobileGL;
using namespace MobileGL::MG_Impl::GLImpl;

class ProgramTest : public ::testing::Test {
protected:
    void SetUp() override { MG_State::pGLContext = new MG_State::GLState::GLContext(); }

    void TearDown() override { delete MG_State::pGLContext; }
};

TEST_F(ProgramTest, Sanity) {
    ASSERT_TRUE(true);
}

const char* vsSrc = R"(#version 460

layout (location = 0) in vec4 Position;
in float fIn4;
in float fIn2;
in float fIn5;
in float fIn6;
in float fIn1;
in float fIn3;

layout(location = 0) uniform mat4 ProjMat;
layout(location = 10) uniform mat3 TestMat3;
layout(location = 20) uniform mat2 TestMat2;
uniform vec2 InSize;
uniform vec2 OutSize;

out vec2 texCoord;
out vec2 oneTexel;

void main(){
    vec4 outPos = ProjMat * vec4(Position.xy, 0.0, 1.0);
    gl_Position = vec4(outPos.xy, 0.2, 1.0);

    // Use TestMat2 and TestMat3 to prevent optimization
    vec2 dummy2 = TestMat2[0];
    vec3 dummy3 = TestMat3[0];
    
    oneTexel = (1.0 * (fIn1 * fIn2 * fIn3 * fIn4 * fIn5 * fIn6)) / InSize;

    texCoord = Position.xy / OutSize;
})";

const char* fsSrc = R"(#version 460

uniform sampler2D InSampler;

in vec2 texCoord;
in vec2 oneTexel;

uniform vec2 InSize;

layout(location = 1) uniform vec3 Gray;
uniform vec3 RedMatrix;
uniform vec3 GreenMatrix0;
uniform vec3 BlueMatrix;
uniform vec3 Offset;
uniform vec3 ColorScale;
layout(location = 6) uniform float Saturation;
uniform int AQuickFoxJumpsOverALazyDog;
uniform int intVal;

out vec4 fragColor;

void main() {
    vec4 InTexel = texture(InSampler, texCoord);

    // Color Matrix
    float RedValue = dot(InTexel.rgb, RedMatrix);
    float GreenValue = dot(InTexel.rgb, GreenMatrix0);
    float BlueValue = dot(InTexel.rgb, BlueMatrix);
    vec3 OutColor = vec3(RedValue, GreenValue, BlueValue);

    // Offset & Scale
    OutColor = (OutColor * ColorScale) + Offset;

    // Saturation
    float Luma = dot(OutColor, Gray);
    vec3 Chroma = OutColor - Luma;
    OutColor = (Chroma * Saturation) + Luma;

    fragColor = vec4(OutColor, float(intVal));
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
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    printf("Compiling vertex shader: %s\n", vsSrc);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;
    printf("Compiled vertex shader.\n");

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    printf("Compiling fragment shader: %s\n", fsSrc);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;
    printf("Compiled fragment shader.\n");

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    BindAttribLocation(program, 1, "fIn1");
    BindAttribLocation(program, 3, "fIn3");
    BindAttribLocation(program, 5, "fIn5");
    printf("Linking program...\n");
    LinkProgram(program);
    printf("Program linked.\n");

    ASSERT_EQ(GetUniformLocation(program, "ProjMat"), 0);
    ASSERT_EQ(GetUniformLocation(program, "Gray"), 1);
    ASSERT_EQ(GetUniformLocation(program, "Saturation"), 6);
    GLint uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_EQ(uniformCount, 14);
    GLint uniformNameMaxLength = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameMaxLength);
    ASSERT_EQ(uniformNameMaxLength, 12);

    ASSERT_EQ(GetAttribLocation(program, "Position"), 0);
    ASSERT_EQ(GetAttribLocation(program, "fIn1"), 1);
    ASSERT_EQ(GetAttribLocation(program, "fIn3"), 3);
    ASSERT_EQ(GetAttribLocation(program, "fIn5"), 5);

    UseProgram(program);

    auto locRed = GetUniformLocation(program, "RedMatrix");
    Uniform3f(locRed, 1.0, 3.0, 5.0);
    float redVal[3];
    GetUniformfv(program, locRed, redVal);
    ASSERT_EQ(redVal[0], 1.0);
    ASSERT_EQ(redVal[1], 3.0);
    ASSERT_EQ(redVal[2], 5.0);

    auto locAbc = GetUniformLocation(program, "AQuickFoxJumpsOverALazyDog");
    ASSERT_EQ(locAbc, -1);

    auto locInt = GetUniformLocation(program, "intVal");
    Uniform1i(locInt, 114514);
    int intVal;
    GetUniformiv(program, locInt, &intVal);
    EXPECT_EQ(intVal, 114514);
}

TEST_F(ProgramTest, UniformMatrixFunctions) {
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    printf("Compiling vertex shader: %s\n", vsSrc);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;
    printf("Compiled vertex shader.\n");

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    printf("Compiling fragment shader: %s\n", fsSrc);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;
    printf("Compiled fragment shader.\n");

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    BindAttribLocation(program, 1, "fIn1");
    BindAttribLocation(program, 3, "fIn3");
    BindAttribLocation(program, 5, "fIn5");
    printf("Linking program...\n");
    LinkProgram(program);
    printf("Program linked.\n");

    UseProgram(program);

    int uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_LT(uniformCount, 4000);

    // Test UniformMatrix2fv
    auto locProjMat = GetUniformLocation(program, "ProjMat");
    ASSERT_NE(locProjMat, -1);

    // 4x4 matrix (16 elements) - identity matrix
    GLfloat matrix4x4[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    // Test UniformMatrix4fv with count = 1 and transpose = GL_FALSE
    UniformMatrix4fv(locProjMat, 1, GL_FALSE, matrix4x4);

    // Test UniformMatrix4fv with count = 1 and transpose = GL_TRUE
    UniformMatrix4fv(locProjMat, 1, GL_TRUE, matrix4x4);

    // Test with a non-identity matrix
    GLfloat nonIdentityMatrix[16] = {1.0f, 2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,
                                     9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    // Test with transpose = GL_FALSE
    UniformMatrix4fv(locProjMat, 1, GL_FALSE, nonIdentityMatrix);

    // Test with transpose = GL_TRUE
    UniformMatrix4fv(locProjMat, 1, GL_TRUE, nonIdentityMatrix);
}

TEST_F(ProgramTest, UniformMatrixTranspose) {
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    printf("Compiling vertex shader: %s\n", vsSrc);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;
    printf("Compiled vertex shader.\n");

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    printf("Compiling fragment shader: %s\n", fsSrc);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;
    printf("Compiled fragment shader.\n");

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    BindAttribLocation(program, 1, "fIn1");
    BindAttribLocation(program, 3, "fIn3");
    BindAttribLocation(program, 5, "fIn5");
    printf("Linking program...\n");
    LinkProgram(program);
    printf("Program linked.\n");

    UseProgram(program);

    int uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_LT(uniformCount, 4000);

    // Test 2x2 matrix transpose
    auto locMat2 = GetUniformLocation(program, "TestMat2");
    ASSERT_NE(locMat2, -1);
    // Test matrix (column-major as expected by OpenGL):
    // [1  3]
    // [2  4]
    GLfloat matrix2x2[4] = {
        1.0f, 2.0f, // First column
        3.0f, 4.0f  // Second column
    };

    // Expected values when transpose = GL_FALSE (no transpose):
    // [1  3]
    // [2  4]
    GLfloat expected2x2_no_transpose[4] = {1.0f, 2.0f, 3.0f, 4.0f};

    // Expected values when transpose = GL_TRUE (transposed):
    // [1  2]
    // [3  4]
    // Stored in column-major order: [1, 3, 2, 4]
    GLfloat expected2x2_transpose[4] = {1.0f, 3.0f, 2.0f, 4.0f};

    // Test with transpose = GL_FALSE
    UniformMatrix2fv(locMat2, 1, GL_FALSE, matrix2x2);
    GLfloat result2x2_no_transpose[4];
    GetUniformfv(program, locMat2, result2x2_no_transpose);
    for (int i = 0; i < 4; i++) {
        EXPECT_FLOAT_EQ(result2x2_no_transpose[i], expected2x2_no_transpose[i]);
    }

    // Test with transpose = GL_TRUE
    UniformMatrix2fv(locMat2, 1, GL_TRUE, matrix2x2);
    GLfloat result2x2_transpose[4];
    GetUniformfv(program, locMat2, result2x2_transpose);
    for (int i = 0; i < 4; i++) {
        EXPECT_FLOAT_EQ(result2x2_transpose[i], expected2x2_transpose[i]);
    }

    // Test 3x3 matrix transpose
    auto locMat3 = GetUniformLocation(program, "TestMat3");
    ASSERT_NE(locMat3, -1);
    // Test matrix (column-major as expected by OpenGL):
    // [1  4  7]
    // [2  5  8]
    // [3  6  9]
    GLfloat matrix3x3[9] = {
        1.0f, 2.0f, 3.0f, // First column
        4.0f, 5.0f, 6.0f, // Second column
        7.0f, 8.0f, 9.0f  // Third column
    };

    // Expected values when transpose = GL_FALSE (no transpose):
    // [1  4  7]
    // [2  5  8]
    // [3  6  9]
    GLfloat expected3x3_no_transpose[9] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

    // Expected values when transpose = GL_TRUE (transposed):
    // [1  2  3]
    // [4  5  6]
    // [7  8  9]
    // Stored in column-major order: [1, 4, 7, 2, 5, 8, 3, 6, 9]
    GLfloat expected3x3_transpose[9] = {1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f};

    // Test with transpose = GL_FALSE
    UniformMatrix3fv(locMat3, 1, GL_FALSE, matrix3x3);
    GLfloat result3x3_no_transpose[9];
    GetUniformfv(program, locMat3, result3x3_no_transpose);
    for (int i = 0; i < 9; i++) {
        EXPECT_FLOAT_EQ(result3x3_no_transpose[i], expected3x3_no_transpose[i]);
    }

    // Test with transpose = GL_TRUE
    UniformMatrix3fv(locMat3, 1, GL_TRUE, matrix3x3);
    GLfloat result3x3_transpose[9];
    GetUniformfv(program, locMat3, result3x3_transpose);
    for (int i = 0; i < 9; i++) {
        EXPECT_FLOAT_EQ(result3x3_transpose[i], expected3x3_transpose[i]);
    }

    // Test 4x4 matrix transpose
    auto locProjMat = GetUniformLocation(program, "ProjMat");
    ASSERT_NE(locProjMat, -1);

    // Test matrix (column-major as expected by OpenGL):
    // [1  5  9  13]
    // [2  6  10 14]
    // [3  7  11 15]
    // [4  8  12 16]
    GLfloat matrix4x4[16] = {
        1.0f,  2.0f,  3.0f,  4.0f,  // First column
        5.0f,  6.0f,  7.0f,  8.0f,  // Second column
        9.0f,  10.0f, 11.0f, 12.0f, // Third column
        13.0f, 14.0f, 15.0f, 16.0f  // Fourth column
    };

    // Expected values when transpose = GL_FALSE (no transpose):
    // [1  5  9  13]
    // [2  6  10 14]
    // [3  7  11 15]
    // [4  8  12 16]
    GLfloat expected4x4_no_transpose[16] = {1.0f, 2.0f,  3.0f,  4.0f,  5.0f,  6.0f,  7.0f,  8.0f,
                                            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};

    // Expected values when transpose = GL_TRUE (transposed):
    // [1  2  3  4]
    // [5  6  7  8]
    // [9  10 11 12]
    // [13 14 15 16]
    // Stored in column-major order
    GLfloat expected4x4_transpose[16] = {1.0f, 5.0f, 9.0f,  13.0f, 2.0f, 6.0f, 10.0f, 14.0f,
                                         3.0f, 7.0f, 11.0f, 15.0f, 4.0f, 8.0f, 12.0f, 16.0f};

    // Test with transpose = GL_FALSE
    UniformMatrix4fv(locProjMat, 1, GL_FALSE, matrix4x4);
    GLfloat result4x4_no_transpose[16];
    GetUniformfv(program, locProjMat, result4x4_no_transpose);
    for (int i = 0; i < 16; i++) {
        EXPECT_FLOAT_EQ(result4x4_no_transpose[i], expected4x4_no_transpose[i]);
    }

    // Test with transpose = GL_TRUE
    UniformMatrix4fv(locProjMat, 1, GL_TRUE, matrix4x4);
    GLfloat result4x4_transpose[16];
    GetUniformfv(program, locProjMat, result4x4_transpose);
    for (int i = 0; i < 16; i++) {
        EXPECT_FLOAT_EQ(result4x4_transpose[i], expected4x4_transpose[i]);
    }
}

TEST_F(ProgramTest, UniformLocationGaps) {
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &vsSrc, NULL);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &fsSrc, NULL);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    BindAttribLocation(program, 1, "fIn1");
    BindAttribLocation(program, 3, "fIn3");
    BindAttribLocation(program, 5, "fIn5");
    LinkProgram(program);

    UseProgram(program);

    int uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_LT(uniformCount, 4000);

    // Test that uniform locations are correctly assigned even with gaps
    // ProjMat is at location 0
    ASSERT_EQ(GetUniformLocation(program, "ProjMat"), 0);

    // TestMat3 is at location 10 (gap from 1-9)
    ASSERT_EQ(GetUniformLocation(program, "TestMat3"), 10);

    // TestMat2 is at location 20 (gap from 11-19)
    ASSERT_EQ(GetUniformLocation(program, "TestMat2"), 20);

    // Gray is at location 1 (no gap)
    ASSERT_EQ(GetUniformLocation(program, "Gray"), 1);

    // Saturation is at location 6 (gap from 2-5)
    ASSERT_EQ(GetUniformLocation(program, "Saturation"), 6);

    // Verify that locations in gaps correctly return -1
    ASSERT_EQ(GetUniformLocation(program, "NonExistentUniform"), -1);

    // Test uniform operations on locations with gaps
    GLfloat matrix3[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    // Test setting and getting uniform at location 10 (TestMat3)
    UniformMatrix3fv(10, 1, GL_FALSE, matrix3);
    GLfloat result[9];
    GetUniformfv(program, 10, result);
    for (int i = 0; i < 9; i++) {
        EXPECT_FLOAT_EQ(result[i], matrix3[i]);
    }

    // Test setting and getting uniform at location 20 (TestMat2)
    GLfloat matrix2[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    UniformMatrix2fv(20, 1, GL_FALSE, matrix2);
    GLfloat result2[4];
    GetUniformfv(program, 20, result2);
    for (int i = 0; i < 4; i++) {
        EXPECT_FLOAT_EQ(result2[i], matrix2[i]);
    }

    // Test that accessing a gap location (e.g., 5) doesn't cause issues
    // This should not crash or cause undefined behavior
    Uniform1i(5, 114514); // Just to make sure we don't crash

    // Verify that we can still use uniforms with sequential locations
    auto locRed = GetUniformLocation(program, "RedMatrix");
    Uniform3f(locRed, 1.0, 3.0, 5.0);
    float redVal[3];
    GetUniformfv(program, locRed, redVal);
    ASSERT_EQ(redVal[0], 1.0);
    ASSERT_EQ(redVal[1], 3.0);
    ASSERT_EQ(redVal[2], 5.0);
}

const char* mc_position_tex_fs = R"(#version 150

uniform sampler2D Sampler0;

uniform vec4 ColorModulator;

in vec2 texCoord0;

out vec4 fragColor;

void main() {
    vec4 color = texture(Sampler0, texCoord0);
    if (color.a == 0.0) {
        discard;
    }
    fragColor = color * ColorModulator;
}
)";

const char* mc_position_tex_vs = R"(#version 150

in vec3 Position;
in vec2 UV0;

uniform mat4 ModelViewMat;
uniform mat4 ProjMat;

out vec2 texCoord0;

void main() {
    gl_Position = ProjMat * ModelViewMat * vec4(Position, 1.0);

    texCoord0 = UV0;
}
)";

TEST_F(ProgramTest, MinecraftPositionTex) {
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &mc_position_tex_vs, NULL);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &mc_position_tex_fs, NULL);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    LinkProgram(program);

    UseProgram(program);

    int uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_LT(uniformCount, 4000);

    int sampler0Loc = GetUniformLocation(program, "Sampler0");
    ASSERT_GE(sampler0Loc, 0);
    ASSERT_LT(sampler0Loc, 4000);
}

const char* minecraft_core_blit_screen_vs = R"(#version 150

in vec3 Position;

out vec2 texCoord;

void main() {
    vec2 screenPos = Position.xy * 2.0 - 1.0;
    gl_Position = vec4(screenPos.x, screenPos.y, 1.0, 1.0);
    texCoord = Position.xy;
}

)";

const char* minecraft_core_lightmap = R"(#version 150

uniform float AmbientLightFactor;
uniform float SkyFactor;
uniform float BlockFactor;
uniform int UseBrightLightmap;
uniform vec3 SkyLightColor;
uniform float NightVisionFactor;
uniform float DarknessScale;
uniform float DarkenWorldFactor;
uniform float BrightnessFactor;

in vec2 texCoord;

out vec4 fragColor;

float get_brightness(float level) {
    float curved_level = level / (4.0 - 3.0 * level);
    return mix(curved_level, 1.0, AmbientLightFactor);
}

vec3 notGamma(vec3 x) {
    vec3 nx = 1.0 - x;
    return 1.0 - nx * nx * nx * nx;
}

void main() {
    float block_brightness = get_brightness(floor(texCoord.x * 16) / 15) * BlockFactor;
    float sky_brightness = get_brightness(floor(texCoord.y * 16) / 15) * SkyFactor;

    // cubic nonsense, dips to yellowish in the middle, white when fully saturated
    vec3 color = vec3(
        block_brightness,
        block_brightness * ((block_brightness * 0.6 + 0.4) * 0.6 + 0.4),
        block_brightness * (block_brightness * block_brightness * 0.6 + 0.4)
    );

    if (UseBrightLightmap != 0) {
        color = mix(color, vec3(0.99, 1.12, 1.0), 0.25);
        color = clamp(color, 0.0, 1.0);
    } else {
        color += SkyLightColor * sky_brightness;
        color = mix(color, vec3(0.75), 0.04);

        vec3 darkened_color = color * vec3(0.7, 0.6, 0.6);
        color = mix(color, darkened_color, DarkenWorldFactor);
    }

    if (NightVisionFactor > 0.0) {
        // scale up uniformly until 1.0 is hit by one of the colors
        float max_component = max(color.r, max(color.g, color.b));
        if (max_component < 1.0) {
            vec3 bright_color = color / max_component;
            color = mix(color, bright_color, NightVisionFactor);
        }
    }

    if (UseBrightLightmap == 0) {
        color = clamp(color - vec3(DarknessScale), 0.0, 1.0);
    }

    vec3 notGamma = notGamma(color);
    color = mix(color, notGamma, BrightnessFactor);
    color = mix(color, vec3(0.75), 0.04);
    color = clamp(color, 0.0, 1.0);

    fragColor = vec4(color, 1.0);
}

)";

TEST_F(ProgramTest, MinecraftBlitScreenLightmap) {
    char infoLog[1024] = "";

    GLuint vs = CreateShader(GL_VERTEX_SHADER);
    ShaderSource(vs, 1, &minecraft_core_blit_screen_vs, NULL);
    CompileShader(vs);
    GLint vsStatus = GL_FALSE;
    GetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    GetShaderInfoLog(vs, 1024, nullptr, infoLog);
    ASSERT_EQ(vsStatus, GL_TRUE) << infoLog;

    GLuint fs = CreateShader(GL_FRAGMENT_SHADER);
    ShaderSource(fs, 1, &minecraft_core_lightmap, NULL);
    CompileShader(fs);
    GLint fsStatus = GL_FALSE;
    GetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);
    GetShaderInfoLog(fs, 1024, nullptr, infoLog);
    ASSERT_EQ(fsStatus, GL_TRUE) << infoLog;

    GLuint program = CreateProgram();
    AttachShader(program, vs);
    AttachShader(program, fs);

    LinkProgram(program);

    UseProgram(program);

    int uniformCount = 0;
    GetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    ASSERT_LT(uniformCount, 4000);

    int loc = GetUniformLocation(program, "AmbientLightFactor");
    ASSERT_GE(loc, 0);
    ASSERT_LT(loc, 4000);

    auto programObject = MG_State::pGLContext->GetCurrentProgram();
    ASSERT_GT(programObject->GetUBOSize(), 0);
}

