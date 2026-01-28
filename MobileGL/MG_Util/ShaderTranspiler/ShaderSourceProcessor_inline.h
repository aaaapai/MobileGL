#pragma once
#include <Includes.h>
#include <MG_State/GLState/ProgramState/ShaderObject.h>

namespace MobileGL {

   namespace MG_Util {
        namespace ShaderTranspiler {

// 综合GLSL代码注入和替换函数
static inline void inject_glsl_replacements(ShaderStage stage, std::string& source) {

    // ==================== 2. 注入gl_DepthRange替换 ====================
    const char* str_depthrange = "gl_DepthRange";
    if (source.find(str_depthrange) != String::npos) {
        // 检查是否已经定义了mg_DepthRange
        const char* str_mg_depthrange_def = "mg_DepthRangeParameters mg_DepthRange;";
        if (source.find(str_mg_depthrange_def) == String::npos) {
            // 替换所有gl_DepthRange为mg_DepthRange
            SizeT depthrangePos = source.find(str_depthrange);
            while (depthrangePos != String::npos) {
                source = source.replace(depthrangePos, strlen(str_depthrange), "mg_DepthRange");
                depthrangePos = source.find(str_depthrange);
            }
            
            // 在#version后插入mg_DepthRange定义
            const std::string gl_DepthRangeImpl = R"(
struct mg_DepthRangeParameters {
    float near;
    float far;
    float diff;
};
uniform mg_DepthRangeParameters mg_DepthRange;
)";
            
            // 找到插入点（在#version行之后）
            SizeT versionPos = source.find("#version");
            if (versionPos != String::npos) {
                SizeT lineEnd = source.find("\n", versionPos);
                if (lineEnd != String::npos) {
                    source.insert(lineEnd + 1, "\n" + gl_DepthRangeImpl + "\n");
                }
            }
        }
    }

    // ==================== 3. 注入subgroup_BigGiftPackage ====================
    // 检查是否使用了任何subgroup相关标识符
    bool hasSubgroupIdentifiers = false;
    const char* subgroupKeywords[] = {
        "subgroupBallot", "activeMask", "gl_SubgroupInvocationID",
        "subgroupAdd", "gl_NumSubgroups", "gl_SubgroupID",
        "subgroupAny", "subgroupAll", "subgroupElect",
        "subgroupExclusiveAdd", "subgroupExclusiveMax", "gl_SubgroupSize"
    };
    
    for (const char* keyword : subgroupKeywords) {
        if (source.find(keyword) != String::npos) {
            hasSubgroupIdentifiers = true;
            break;
        }
    }
    
    if (hasSubgroupIdentifiers) {
        // 检查是否已经定义了SUBGROUP_SIZE
        const char* str_subgroup_size = "#define SUBGROUP_SIZE";
        if (source.find(str_subgroup_size) == String::npos) {
            // 注释掉所有KHR_shader_subgroup扩展
            const char* extensionKeywords[] = {
                "#extension GL_KHR_shader_subgroup",
                "#extension GL_KHR_shader_subgroup_basic",
                "#extension GL_KHR_shader_subgroup_ballot",
                "#extension GL_KHR_shader_subgroup_arithmetic",
                "#extension GL_KHR_shader_subgroup_clustered"
            };
            
            for (const char* ext : extensionKeywords) {
                SizeT extPos = source.find(ext);
                while (extPos != String::npos) {
                    source = source.replace(extPos, strlen(ext), (std::string("// ") + ext).c_str());
                    extPos = source.find(ext);
                }
            }
            
            // 替换所有subgroup相关标识符
            const char* replacePairs[][2] = {
                {"subgroupBallot", "mg_subgroupBallot"},
                {"activeMask", "mg_activeMask"},
                {"gl_SubgroupInvocationID", "mg_gl_SubgroupInvocationID()"},
                {"subgroupAdd", "mg_subgroupAdd"},
                {"gl_NumSubgroups", "mg_gl_NumSubgroups()"},
                {"gl_SubgroupID", "mg_gl_SubgroupID()"},
                {"subgroupAny", "mg_subgroupAny"},
                {"subgroupAll", "mg_subgroupAll"},
                {"subgroupElect", "mg_subgroupElect"},
                {"subgroupExclusiveAdd", "mg_subgroupExclusiveAdd"},
                {"subgroupExclusiveMax", "mg_subgroupExclusiveMax"},
                {"gl_SubgroupSize", "mg_gl_SubgroupSize()"}
            };
            
            for (const auto& pair : replacePairs) {
                const char* oldStr = pair[0];
                const char* newStr = pair[1];
                SizeT pos = source.find(oldStr);
                while (pos != String::npos) {
                    source = source.replace(pos, strlen(oldStr), newStr);
                    pos = source.find(oldStr);
                }
            }
            
            // 插入subgroup_BigGiftPackage实现（完整的模拟实现代码）
            const std::string subgroupImpl = R"(
#define SUBGROUP_SIZE 32

// ==================== 基础子组模拟 ====================
uint mg_gl_SubgroupID() {
    return gl_LocalInvocationIndex / SUBGROUP_SIZE;
}

uint mg_gl_SubgroupInvocationID() {
    return gl_LocalInvocationIndex % SUBGROUP_SIZE;
}

uint mg_gl_NumSubgroups() {
    return (gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + SUBGROUP_SIZE - 1u) / SUBGROUP_SIZE;
}

uint mg_gl_SubgroupSize() {
    return SUBGROUP_SIZE;
}

// ==================== 投票功能模拟 ====================
shared uint s_ballot;

uvec2 mg_subgroupBallot(bool condition) {
    if (gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0 && gl_LocalInvocationID.z == 0) {
        s_ballot = 0u;
    }
    memoryBarrierShared();
    barrier();

    if (condition) {
        uint mask = 1u << mg_gl_SubgroupInvocationID();
        atomicOr(s_ballot, mask);
    }
    memoryBarrierShared();
    barrier();

    return uvec2(s_ballot, 0u);
}

uvec2 activeMask() {
    return mg_subgroupBallot(true);
}

bool mg_subgroupAny(bool value) {
    uvec2 mask = mg_subgroupBallot(value);
    return mask.x != 0u;
}

bool mg_subgroupAll(bool value) {
    uvec2 fullMask = activeMask();
    uvec2 valueMask = mg_subgroupBallot(value);
    return fullMask == valueMask;
}

bool mg_subgroupElect() {
    return (mg_gl_SubgroupInvocationID() == 0u);
}

// ==================== 全局共享内存声明 ====================
const uint total_workgroup_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
const uint num_subgroups = (total_workgroup_size + SUBGROUP_SIZE - 1u) / SUBGROUP_SIZE;

shared float s_reduceAdd_float[num_subgroups * SUBGROUP_SIZE];
shared uint s_reduceAdd_uint[num_subgroups * SUBGROUP_SIZE];
shared int s_reduceAdd_int[num_subgroups * SUBGROUP_SIZE];
shared vec2 s_reduceAdd_vec2[num_subgroups * SUBGROUP_SIZE];
shared vec3 s_reduceAdd_vec3[num_subgroups * SUBGROUP_SIZE];
shared vec4 s_reduceAdd_vec4[num_subgroups * SUBGROUP_SIZE];

shared float s_exclusiveAdd_float[num_subgroups * SUBGROUP_SIZE];
shared uint s_exclusiveAdd_uint[num_subgroups * SUBGROUP_SIZE];
shared int s_exclusiveAdd_int[num_subgroups * SUBGROUP_SIZE];
shared float s_exclusiveMax_float[num_subgroups * SUBGROUP_SIZE];
shared uint s_exclusiveMax_uint[num_subgroups * SUBGROUP_SIZE];
shared int s_exclusiveMax_int[num_subgroups * SUBGROUP_SIZE];
shared vec2 s_exclusiveAdd_vec2[num_subgroups * SUBGROUP_SIZE];
shared vec3 s_exclusiveAdd_vec3[num_subgroups * SUBGROUP_SIZE];
shared vec4 s_exclusiveAdd_vec4[num_subgroups * SUBGROUP_SIZE];
shared vec2 s_exclusiveMax_vec2[num_subgroups * SUBGROUP_SIZE];
shared vec3 s_exclusiveMax_vec3[num_subgroups * SUBGROUP_SIZE];
shared vec4 s_exclusiveMax_vec4[num_subgroups * SUBGROUP_SIZE];

// ==================== 子组加法归约模拟 ====================
float mg_subgroupAdd(float value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_float[i] = 0.0;
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_float[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_float[offset] += s_reduceAdd_float[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_float[subgroupID * SUBGROUP_SIZE];
}

uint mg_subgroupAdd(uint value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_uint[i] = 0u;
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_uint[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_uint[offset] += s_reduceAdd_uint[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_uint[subgroupID * SUBGROUP_SIZE];
}

int mg_subgroupAdd(int value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_int[i] = 0;
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_int[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_int[offset] += s_reduceAdd_int[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_int[subgroupID * SUBGROUP_SIZE];
}

vec2 mg_subgroupAdd(vec2 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_vec2[i] = vec2(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_vec2[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_vec2[offset] += s_reduceAdd_vec2[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_vec2[subgroupID * SUBGROUP_SIZE];
}

vec3 mg_subgroupAdd(vec3 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_vec3[i] = vec3(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_vec3[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_vec3[offset] += s_reduceAdd_vec3[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_vec3[subgroupID * SUBGROUP_SIZE];
}

vec4 mg_subgroupAdd(vec4 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_reduceAdd_vec4[i] = vec4(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_reduceAdd_vec4[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = SUBGROUP_SIZE / 2; stride > 0; stride >>= 1) {
        if (laneID < stride) {
            s_reduceAdd_vec4[offset] += s_reduceAdd_vec4[offset + stride];
        }
        memoryBarrierShared();
        barrier();
    }

    return s_reduceAdd_vec4[subgroupID * SUBGROUP_SIZE];
}

// ==================== 子组排他性加法模拟 ====================
float mg_subgroupExclusiveAdd(float value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_float[i] = 0.0;
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_float[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        float temp = 0.0;
        if (laneID >= stride) {
            temp = s_exclusiveAdd_float[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_float[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return 0.0;
    } else {
        return s_exclusiveAdd_float[offset - 1];
    }
}

uint mg_subgroupExclusiveAdd(uint value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_uint[i] = 0u;
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_uint[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        uint temp = 0u;
        if (laneID >= stride) {
            temp = s_exclusiveAdd_uint[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_uint[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return 0u;
    } else {
        return s_exclusiveAdd_uint[offset - 1];
    }
}

int mg_subgroupExclusiveAdd(int value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_int[i] = 0;
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_int[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        int temp = 0;
        if (laneID >= stride) {
            temp = s_exclusiveAdd_int[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_int[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return 0;
    } else {
        return s_exclusiveAdd_int[offset - 1];
    }
}

vec2 mg_subgroupExclusiveAdd(vec2 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_vec2[i] = vec2(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_vec2[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec2 temp = vec2(0.0);
        if (laneID >= stride) {
            temp = s_exclusiveAdd_vec2[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_vec2[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec2(0.0);
    } else {
        return s_exclusiveAdd_vec2[offset - 1];
    }
}

vec3 mg_subgroupExclusiveAdd(vec3 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_vec3[i] = vec3(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_vec3[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec3 temp = vec3(0.0);
        if (laneID >= stride) {
            temp = s_exclusiveAdd_vec3[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_vec3[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec3(0.0);
    } else {
        return s_exclusiveAdd_vec3[offset - 1];
    }
}

vec4 mg_subgroupExclusiveAdd(vec4 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveAdd_vec4[i] = vec4(0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveAdd_vec4[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec4 temp = vec4(0.0);
        if (laneID >= stride) {
            temp = s_exclusiveAdd_vec4[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveAdd_vec4[offset] += temp;
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec4(0.0);
    } else {
        return s_exclusiveAdd_vec4[offset - 1];
    }
}

// ==================== 子组排他性最大值模拟 ====================
float mg_subgroupExclusiveMax(float value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_float[i] = -1.0 / 0.0;
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_float[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        float temp = -1.0 / 0.0;
        if (laneID >= stride) {
            temp = s_exclusiveMax_float[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_float[offset] = max(s_exclusiveMax_float[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return -1.0 / 0.0;
    } else {
        return s_exclusiveMax_float[offset - 1];
    }
}

uint mg_subgroupExclusiveMax(uint value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_uint[i] = 0u;
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_uint[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        uint temp = 0u;
        if (laneID >= stride) {
            temp = s_exclusiveMax_uint[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_uint[offset] = max(s_exclusiveMax_uint[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return 0u;
    } else {
        return s_exclusiveMax_uint[offset - 1];
    }
}

int mg_subgroupExclusiveMax(int value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_int[i] = (-1 << 31);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_int[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        int temp = (-1 << 31);
        if (laneID >= stride) {
            temp = s_exclusiveMax_int[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_int[offset] = max(s_exclusiveMax_int[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return (-1 << 31);
    } else {
        return s_exclusiveMax_int[offset - 1];
    }
}

vec2 mg_subgroupExclusiveMax(vec2 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_vec2[i] = vec2(-1.0 / 0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_vec2[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec2 temp = vec2(-1.0 / 0.0);
        if (laneID >= stride) {
            temp = s_exclusiveMax_vec2[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_vec2[offset] = max(s_exclusiveMax_vec2[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec2(-1.0 / 0.0);
    } else {
        return s_exclusiveMax_vec2[offset - 1];
    }
}

vec3 mg_subgroupExclusiveMax(vec3 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_vec3[i] = vec3(-1.0 / 0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_vec3[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec3 temp = vec3(-1.0 / 0.0);
        if (laneID >= stride) {
            temp = s_exclusiveMax_vec3[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_vec3[offset] = max(s_exclusiveMax_vec3[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec3(-1.0 / 0.0);
    } else {
        return s_exclusiveMax_vec3[offset - 1];
    }
}

vec4 mg_subgroupExclusiveMax(vec4 value) {
    uint subgroupID = mg_gl_SubgroupID();
    uint laneID = mg_gl_SubgroupInvocationID();
    uint offset = subgroupID * SUBGROUP_SIZE + laneID;

    for (uint i = gl_LocalInvocationIndex; i < num_subgroups * SUBGROUP_SIZE; i += total_workgroup_size) {
        s_exclusiveMax_vec4[i] = vec4(-1.0 / 0.0);
    }
    memoryBarrierShared();
    barrier();

    s_exclusiveMax_vec4[offset] = value;
    memoryBarrierShared();
    barrier();

    for (uint stride = 1; stride < SUBGROUP_SIZE; stride <<= 1) {
        vec4 temp = vec4(-1.0 / 0.0);
        if (laneID >= stride) {
            temp = s_exclusiveMax_vec4[offset - stride];
        }
        memoryBarrierShared();
        barrier();
        if (laneID >= stride) {
            s_exclusiveMax_vec4[offset] = max(s_exclusiveMax_vec4[offset], temp);
        }
        memoryBarrierShared();
        barrier();
    }

    if (laneID == 0) {
        return vec4(-1.0 / 0.0);
    } else {
        return s_exclusiveMax_vec4[offset - 1];
    }
}
)";
            
            // 在uniform声明后或void main()前插入
            SizeT mainPos = source.find("void main()");
            if (mainPos != String::npos) {
                // 在main函数前插入
                source.insert(mainPos, "\n" + subgroupImpl + "\n");
            } else {
                // 否则在文件末尾插入
                source += "\n" + subgroupImpl + "\n";
            }
        }
    }

    // ==================== 4. 注入subgroup_clustered ====================
    // 检查是否使用了subgroupClustered相关标识符
    bool hasClusteredIdentifiers = false;
    const char* clusteredKeywords[] = {
        "subgroupClusteredMax", "subgroupMemoryBarrier", "subgroupBarrier",
        "subgroupClusteredAllEqual", "subgroupClusteredAny", "subgroupClusteredAll",
        "subgroupClusteredXor", "subgroupClusteredOr", "subgroupClusteredAnd",
        "subgroupClusteredMin", "subgroupClusteredMul", "subgroupClusteredAdd"
    };
    
    for (const char* keyword : clusteredKeywords) {
        if (source.find(keyword) != String::npos) {
            hasClusteredIdentifiers = true;
            break;
        }
    }
    
    if (hasClusteredIdentifiers) {
        // 检查是否已经定义了_cluster_shared_data
        const char* str_cluster_data = "_cluster_shared_data[";
        if (source.find(str_cluster_data) == String::npos) {
            // 注释掉所有clustered扩展
            const char* extensionVariants[] = {
                "#extension GL_KHR_shader_subgroup_clustered :enable",
                "#extension GL_KHR_shader_subgroup_clustered : enable",
                "#extension GL_KHR_shader_subgroup_clustered: enable",
                "#extension GL_KHR_shader_subgroup_clustered : require",
                "#extension GL_KHR_shader_subgroup_clustered :require"
            };
            
            for (const char* ext : extensionVariants) {
                SizeT extPos = source.find(ext);
                while (extPos != String::npos) {
                    source = source.replace(extPos, strlen(ext), (std::string("// ") + ext).c_str());
                    extPos = source.find(ext);
                }
            }
            
            // 替换所有clustered相关标识符
            const char* clusteredReplacePairs[][2] = {
                {"subgroupClusteredMax", "mg_subgroupClusteredMax"},
                {"subgroupMemoryBarrier", "mg_subgroupMemoryBarrier"},
                {"subgroupBarrier", "mg_subgroupBarrier"},
                {"subgroupClusteredAllEqual", "mg_subgroupClusteredAllEqual"},
                {"subgroupClusteredAny", "mg_subgroupClusteredAny"},
                {"subgroupClusteredAll", "mg_subgroupClusteredAll"},
                {"subgroupClusteredXor", "mg_subgroupClusteredXor"},
                {"subgroupClusteredOr", "mg_subgroupClusteredOr"},
                {"subgroupClusteredAnd", "mg_subgroupClusteredAnd"},
                {"subgroupClusteredMin", "mg_subgroupClusteredMin"},
                {"subgroupClusteredMul", "mg_subgroupClusteredMul"},
                {"subgroupClusteredAdd", "mg_subgroupClusteredAdd"}
            };
            
            for (const auto& pair : clusteredReplacePairs) {
                const char* oldStr = pair[0];
                const char* newStr = pair[1];
                SizeT pos = source.find(oldStr);
                while (pos != String::npos) {
                    source = source.replace(pos, strlen(oldStr), newStr);
                    pos = source.find(oldStr);
                }
            }
            
            // 插入subgroup_clustered实现
            const std::string clusteredImpl = R"(
precision highp float;
precision highp int;

shared uint _cluster_shared_data[gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z];

uint _get_linear_index() {
    return gl_LocalInvocationID.x;
}

uint _clustered_reduce(uint value, uint clusterSize, uint op) {
    uint idx = _get_linear_index();
    uint clusterIdx = idx / clusterSize;
    uint offset = idx % clusterSize;
    uint base = clusterIdx * clusterSize;

    _cluster_shared_data[idx] = value;
    barrier();
    memoryBarrierShared();

    for (uint stride = 1; stride < clusterSize; stride *= 2) {
        if ((offset & (2u * stride - 1u)) == 0u) {
            uint otherIdx = idx + stride;
            if (offset + stride < clusterSize) {
                uint a = _cluster_shared_data[idx];
                uint b = _cluster_shared_data[otherIdx];
                
                switch (op) {
                    case 0:  a += b; break;
                    case 1:  a *= b; break;
                    case 2:  a = min(a, b); break;
                    case 3:  a = max(a, b); break;
                    case 4:  a &= b; break;
                    case 5:  a |= b; break;
                    case 6:  a ^= b; break;
                    default: break;
                }
                _cluster_shared_data[idx] = a;
            }
        }
        barrier();
        memoryBarrierShared();
    }
    return _cluster_shared_data[base];
}

float mg_subgroupClusteredAdd(float val, uint clusterSize) {
    uint u = floatBitsToUint(val);
    u = _clustered_reduce(u, clusterSize, 0);
    return uintBitsToFloat(u);
}

float mg_subgroupClusteredMul(float val, uint clusterSize) {
    uint u = floatBitsToUint(val);
    u = _clustered_reduce(u, clusterSize, 1);
    return uintBitsToFloat(u);
}

float mg_subgroupClusteredMin(float val, uint clusterSize) {
    uint u = floatBitsToUint(val);
    u = _clustered_reduce(u, clusterSize, 2);
    return uintBitsToFloat(u);
}

float mg_subgroupClusteredMax(float val, uint clusterSize) {
    uint u = floatBitsToUint(val);
    u = _clustered_reduce(u, clusterSize, 3);
    return uintBitsToFloat(u);
}

uint mg_subgroupClusteredAnd(uint val, uint clusterSize) {
    return _clustered_reduce(val, clusterSize, 4);
}

uint mg_subgroupClusteredOr(uint val, uint clusterSize) {
    return _clustered_reduce(val, clusterSize, 5);
}

uint mg_subgroupClusteredXor(uint val, uint clusterSize) {
    return _clustered_reduce(val, clusterSize, 6);
}

bool mg_subgroupClusteredAll(bool condition, uint clusterSize) {
    uint val = condition ? 0xFFFFFFFFu : 0u;
    uint result = _clustered_reduce(val, clusterSize, 4);
    return (result == 0xFFFFFFFFu);
}

bool mg_subgroupClusteredAny(bool condition, uint clusterSize) {
    uint val = condition ? 0xFFFFFFFFu : 0u;
    uint result = _clustered_reduce(val, clusterSize, 5);
    return (result != 0u);
}

bool mg_subgroupClusteredAllEqual(float value, uint clusterSize) {
    uint idx = _get_linear_index();
    uint clusterIdx = idx / clusterSize;
    uint offset = idx % clusterSize;
    uint base = clusterIdx * clusterSize;

    _cluster_shared_data[idx] = floatBitsToUint(value);
    barrier();
    memoryBarrierShared();

    uint ref = _cluster_shared_data[base];
    bool equal = (floatBitsToUint(value) == ref);
    uint u = equal ? 0xFFFFFFFFu : 0u;
    uint result = _clustered_reduce(u, clusterSize, 4);
    
    return (result == 0xFFFFFFFFu);
}

void mg_subgroupBarrier() {
    barrier();
    memoryBarrierShared();
}

void mg_subgroupMemoryBarrier() {
    memoryBarrierShared();
}
)";
            
            // 在subgroup_BigGiftPackage实现之后或void main()前插入
            SizeT mainPos2 = source.find("void main()");
            if (mainPos2 != String::npos) {
                source.insert(mainPos2, "\n" + clusteredImpl + "\n");
            } else {
                source += "\n" + clusteredImpl + "\n";
            }
        }
    }

    const char* arbKeywords[] = {
        "gl_DrawIDARB",
        "gl_BaseVertexARB", 
        "gl_BaseInstanceARB"
    };

    const char* standardKeywords[] = {
        "gl_DrawID",
        "gl_BaseVertex",
        "gl_BaseInstance"
    };

    // 直接替换ARB后缀为标准名
    for (int i = 0; i < 3; i++) {
        size_t pos = 0;
        while ((pos = source.find(arbKeywords[i], pos)) != String::npos) {
            // 简单检查一下是不是独立标识符
            bool isWord = true;
            if (pos > 0) {
                char prev = source[pos - 1];
                isWord = !(isalnum(prev) || prev == '_');
            }
            if (isWord && pos + strlen(arbKeywords[i]) < source.length()) {
                char next = source[pos + strlen(arbKeywords[i])];
                isWord = !(isalnum(next) || next == '_');
            }
        
            if (isWord) {
                source.replace(pos, strlen(arbKeywords[i]), standardKeywords[i]);
                pos += strlen(standardKeywords[i]);
            } else {
                pos += strlen(arbKeywords[i]);
            }
        }
    }

    // ==================== 6. 注入temporal_filter ====================
    /*const char* str_temporal_filter = "GI_TemporalFilter";
    if (source.find(str_temporal_filter) != String::npos) {
        // 检查是否已经定义了GI_TemporalFilter函数
        const char* str_temporal_filter_def = "vec4 GI_TemporalFilter()";
        if (source.find(str_temporal_filter_def) == String::npos) {
            const std::string temporalFilterImpl = R"(
vec4 GI_TemporalFilter() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    uv += taaJitter * pixelSize;
    vec4 currentGI = texture(colortex0, uv);
    float depth = texture(depthtex0, uv).r;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = gbufferProjectionInverse * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = gbufferModelViewInverse * viewPos;
    vec4 prevClipPos = gbufferPreviousProjection * (gbufferPreviousModelView * worldPos);
    prevClipPos /= prevClipPos.w;
    vec2 prevUV = prevClipPos.xy * 0.5 + 0.5;
    vec4 historyGI = texture(colortex1, prevUV);
    float difference = length(currentGI.rgb - historyGI.rgb);
    float thresholdValue = 0.1;
    float adaptiveBlend = mix(0.9, 0.0, smoothstep(thresholdValue, thresholdValue * 2.0, difference));
    vec4 filteredGI = mix(currentGI, historyGI, adaptiveBlend);
    if (difference > thresholdValue * 2.0) {
        filteredGI = currentGI;
    }
    return filteredGI;
}
)";
            
            // 在uniform声明后或void main()前插入
            SizeT mainPos3 = source.find("void main()");
            if (mainPos3 != String::npos) {
                source.insert(mainPos3, "\n" + temporalFilterImpl + "\n");
            } else {
                source += "\n" + temporalFilterImpl + "\n";
            }
        }
    }*/

const char* str_temporal_filter = "GI_TemporalFilter";
    if (source.find(str_temporal_filter) != String::npos) {
        // 检查是否已经定义了GI_TemporalFilter函数
        const char* str_temporal_filter_def = "vec4 GI_TemporalFilter()";
        if (source.find(str_temporal_filter_def) == String::npos) {
            const std::string temporalFilterImpl = R"(

#define GI_RENDER_RESOLUTION	0.5 // i have no any method to control the digit of GI_RENDER_RESOLUTION

vec4 GI_TemporalFilter(){
	vec4 prev = texelFetch(colortex2, texelCoord, 0);
	vec2 coord = texCoord / GI_RENDER_RESOLUTION;


	if (saturate(coord) == coord){
		ivec2 texel = ivec2(coord * screenSize);

		float currDepth = texelFetch(depthtex1, texel, 0).x;

		#ifdef DISTANT_HORIZONS
			bool isDH = false;
			if (currDepth == 1.0){
				currDepth = texelFetch(dhDepthTex0, texel, 0).x;
				isDH = true;
			}
		#endif

		if (currDepth < 1.0){
			vec4 data3 = texelFetch(colortex3, texel, 0);
			vec3 currNormal = DecodeNormal(data3.xy);
			vec3 currViewPos = ViewPos_From_ScreenPos_Raw(coord, currDepth);

			#ifdef DISTANT_HORIZONS
				if (isDH) currViewPos = ViewPos_From_ScreenPos_Raw_DH(coord, currDepth);
			#endif

			vec3 gi = vec3(0.0);
			#if MC_VERSION >= 11605
				#if defined SUNLIGHT_LEAK_FIX && !defined DIMENSION_END 
					if (data3.w > 0.0 || isEyeInWater == 1)
				#endif
					gi = RSM(currViewPos, currNormal);
			#else
				#if defined SUNLIGHT_LEAK_FIX && !defined DIMENSION_END 
					if (currDepth > 0.7 && (data3.w > 0.0 || isEyeInWater == 1))
				#else
					if (currDepth > 0.7)
				#endif
					gi = RSM(currViewPos, currNormal);
			#endif


			#ifdef DISTANT_HORIZONS
				vec2 velocity = CalculateCameraVelocity(coord, currDepth, isDH);
			#else
				vec2 velocity = CalculateCameraVelocity(coord, currDepth);
			#endif

			vec2 pcoord = coord - velocity;
			vec2 prevCoord = clamp(pcoord, vec2(0.0), vec2(1.0) - pixelSize * 2.0);

			if (prevCoord == pcoord){
				prevCoord = prevCoord * GI_RENDER_RESOLUTION * screenSize - 0.5;

				vec2 prevTexel = floor(prevCoord);

				vec3 prevGi = vec3(0.0);
				float weights = 0.0;

				for(float i = 0.0; i <= 1.0; i++){
				for(float j = 0.0; j <= 1.0; j++){
					vec2 sampleTexelcoord = prevTexel + vec2(i, j);

					vec4 prevData = texelFetch(colortex2, ivec2(sampleTexelcoord.x + GI_RENDER_RESOLUTION * screenSize.x, sampleTexelcoord.y), 0);

					vec3 prevNormal = prevData.xyz * 2.0 - 1.0;
					float normalWeight = float(dot(currNormal, prevNormal) > 0.5);

					float currDist = -currViewPos.z;
					currDist = min(currDist, 1000.0);
					float prevDist = prevData.a * 1000.0;

					vec3 cameraVelocity = mat3(gbufferModelView) * (cameraPosition - previousCameraPosition);
					float depthWeight = max(abs(currDist - prevDist - cameraVelocity.z), 0.0);
					depthWeight = exp(-depthWeight / (currDist * 0.1 + 0.1));
					depthWeight = saturate(depthWeight * 2.0);

					float bilinearWeight = (1.0 - abs(prevCoord.x - sampleTexelcoord.x)) * (1.0 - abs(prevCoord.y - sampleTexelcoord.y));

					bilinearWeight *= normalWeight * depthWeight;
					
					prevGi += CurveToLinear(texelFetch(colortex2, ivec2(sampleTexelcoord), 0).rgb) * bilinearWeight;
					weights += bilinearWeight;
				}}

				gi = mix(gi, prevGi, 0.95 * weights);
			}

			prev = vec4(LinearToCurve(gi), 0.0);
		}
	}


	coord.x -= 1.0;

	if (saturate(coord) == coord){
		ivec2 texel = ivec2(floor(coord * screenSize));

		float depth = texelFetch(depthtex1, texel, 0).x;

		#ifdef DISTANT_HORIZONS
			bool isDH = false;
			if (depth == 1.0){
				depth = texelFetch(dhDepthTex0, texel, 0).x;
				isDH = true;
			}
		#endif

		if (depth < 1.0){

			vec3 normal = DecodeNormal(texelFetch(colortex3, texel, 0).xy);

			float dist = LinearDepth_From_ScreenDepth(depth);
			#ifdef DISTANT_HORIZONS
				if(isDH) dist = LinearDepth_From_ScreenDepth_DH(depth);
			#endif
			
			prev = vec4(normal * 0.5 + 0.5, dist * 0.001);
		}else{
			prev = vec4(vec3(0.5), 0.0);
		}
	}
	return prev;
}
)";
         
            // 在uniform声明后或void main()前插入
            SizeT mainPos3 = source.find("void main()");
            if (mainPos3 != String::npos) {
                source.insert(mainPos3, "\n" + temporalFilterImpl + "\n");
            } else {
                source += "\n" + temporalFilterImpl + "\n";
            }
        }
    }

// ==================== 替换已弃用语法和修复问题 ====================
    // 替换attribute/varying关键字
    if (stage == ShaderStage::Vertex) {
        // 替换attribute为in
        size_t pos = 0;
        while ((pos = source.find("attribute", pos)) != String::npos) {
            source.replace(pos, 9, "in");
            pos += 2; // "in"的长度
        }
        
        // 替换varying为out
        pos = 0;
        while ((pos = source.find("varying", pos)) != String::npos) {
            source.replace(pos, 7, "out");
            pos += 3; // "out"的长度
        }
    } else if (stage == ShaderStage::Fragment) {
        // 替换varying为in
        size_t pos = 0;
        while ((pos = source.find("varying", pos)) != String::npos) {
            source.replace(pos, 7, "in");
            pos += 2; // "in"的长度
        }
    }
    
    // 替换texture2D为texture
    size_t pos = 0;
    while ((pos = source.find("texture2D", pos)) != String::npos) {
        source.replace(pos, 9, "texture");
        pos += 7; // "texture"的长度
    }
    
    // 为transpose()函数添加polyfill实现
    const std::string transposeTarget = "const mat3 rotInverse = transpose(rot);";
    const std::string transposeReplacement = "const mat3 rotInverse = mat3(rot[0][0], rot[1][0], rot[2][0], rot[0][1], rot[1][1], rot[2][1], rot[0][2], rot[1][2], rot[2][2]);";
    
    pos = source.find(transposeTarget);
    if (pos != String::npos) {
        source.replace(pos, transposeTarget.length(), transposeReplacement);
    }
    
    // 修复vec3数组声明问题
    const std::string vec3ArrayTarget = "vec3[3](vWorldPos[0] - vWorldPos[1]";
    const std::string vec3ArrayReplacement = "vec4[3](vWorldPos[0] - vWorldPos[1]";
    
    pos = source.find(vec3ArrayTarget);
    if (pos != String::npos) {
        source.replace(pos, vec3ArrayTarget.length(), vec3ArrayReplacement);
    }
    
    // 初始化未初始化的变量
    const std::string reflectionTarget = "vec3 reflection;";
    const std::string reflectionReplacement = "vec3 reflection=vec3(0,0,0);";
    
    pos = source.find(reflectionTarget);
    if (pos != String::npos) {
        source.replace(pos, reflectionTarget.length(), reflectionReplacement);
    }
    
    // 注释掉#error指令以避免编译中断
    pos = 0;
    while ((pos = source.find("#error ", pos)) != String::npos) {
        source.replace(pos, 7, "// #error ");
        pos += 10; // "// #error "的长度
    }



}



}
}
}
