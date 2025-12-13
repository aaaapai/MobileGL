#pragma once
#include <Includes.h>

class MultiDrawElementsBaseVertex {
private:
    // GPU资源句柄
    GLuint m_prefixSumBuffer{0};
    GLuint m_firstIndexBuffer{0};
    GLuint m_baseVertexBuffer{0};
    GLuint m_outputIBO{0};
    GLuint m_computeProgram{0};
    
    // CPU端数据
    std::vector<GLuint> m_prefixSums;          // 前缀和
    GLuint m_totalIndices{0};                  // 总索引数
    
    // 状态标志
    bool m_initialized{false};
    bool m_dataDirty{true};
    
    // 着色器源代码 - 支持多种类型
    static constexpr std::string_view COMPUTE_SHADER_SOURCE_UINT = R"(#version 330 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer Input { 
    uint in_indices[]; 
};
layout(std430, binding = 1) readonly buffer FirstIndex { 
    uint firstIndex[]; 
};
layout(std430, binding = 2) readonly buffer BaseVertex { 
    int baseVertex[]; 
};
layout(std430, binding = 3) readonly buffer Prefix { 
    uint prefixSums[]; 
};
layout(std430, binding = 4) writeonly buffer Output { 
    uint out_indices[]; 
};

void main() {
    uint outIdx = gl_GlobalInvocationID.x;
    uint totalIndices = prefixSums[prefixSums.length() - 1u];
    
    if (outIdx >= totalIndices) {
        return;
    }
    
    // 二分查找批次
    int low = 0;
    int high = int(prefixSums.length() - 1u);
    
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (prefixSums[mid] > outIdx) {
            high = mid; // next [low, mid)
        } else {
            low = mid + 1; // next [mid + 1, high)
        }
    }
    
    uint batchID = uint(low);
    
    // 计算本地索引
    uint batchStart = (batchID == 0u) ? 0u : prefixSums[batchID - 1u];
    uint localIdx = outIdx - batchStart;
    
    // 读取原始索引（firstIndex是元素索引）
    uint inIndex = localIdx + firstIndex[batchID];
    uint originalIndex = in_indices[inIndex];
    
    // 应用基顶点偏移
    uint finalIndex = uint(int(originalIndex) + baseVertex[batchID]);
    
    // 写入输出
    out_indices[outIdx] = finalIndex;
}
)";

    static constexpr std::string_view COMPUTE_SHADER_SOURCE_USHORT = R"(#version 330 core
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer Input { 
    uint in_indices[]; // 实际上是ushort，但以uint形式读取
};
layout(std430, binding = 1) readonly buffer FirstIndex { 
    uint firstIndex[]; 
};
layout(std430, binding = 2) readonly buffer BaseVertex { 
    int baseVertex[]; 
};
layout(std430, binding = 3) readonly buffer Prefix { 
    uint prefixSums[]; 
};
layout(std430, binding = 4) writeonly buffer Output { 
    uint out_indices[]; 
};

// 从打包的ushort数组中提取ushort值
uint extractUShort(uint packedValue, uint indexInUint) {
    if (indexInUint == 0u) {
        return packedValue & 0xFFFFu;
    } else {
        return (packedValue >> 16u) & 0xFFFFu;
    }
}

void main() {
    uint outIdx = gl_GlobalInvocationID.x;
    uint totalIndices = prefixSums[prefixSums.length() - 1u];
    
    if (outIdx >= totalIndices) {
        return;
    }
    
    // 二分查找批次
    int low = 0;
    int high = int(prefixSums.length() - 1u);
    
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (prefixSums[mid] > outIdx) {
            high = mid; // next [low, mid)
        } else {
            low = mid + 1; // next [mid + 1, high)
        }
    }
    
    uint batchID = uint(low);
    
    // 计算本地索引
    uint batchStart = (batchID == 0u) ? 0u : prefixSums[batchID - 1u];
    uint localIdx = outIdx - batchStart;
    
    // 读取原始索引（firstIndex是ushort元素索引）
    uint inIndex = localIdx + firstIndex[batchID];
    
    // 计算在uint数组中的位置和偏移
    uint uintIndex = inIndex / 2u;
    uint offsetInUint = inIndex % 2u;
    
    uint packedValue = in_indices[uintIndex];
    uint originalIndex = extractUShort(packedValue, offsetInUint);
    
    // 应用基顶点偏移
    uint finalIndex = uint(int(originalIndex) + baseVertex[batchID]);
    
    // 写入输出
    out_indices[outIdx] = finalIndex;
}
)";

    static constexpr std::string_view COMPUTE_SHADER_SOURCE_UBYTE = R"(#version 320 es
layout(local_size_x = 64) in;

layout(std430, binding = 0) readonly buffer Input { 
    uint in_indices[]; // 实际上是ubyte，但以uint形式读取
};
layout(std430, binding = 1) readonly buffer FirstIndex { 
    uint firstIndex[]; 
};
layout(std430, binding = 2) readonly buffer BaseVertex { 
    int baseVertex[]; 
};
layout(std430, binding = 3) readonly buffer Prefix { 
    uint prefixSums[]; 
};
layout(std430, binding = 4) writeonly buffer Output { 
    uint out_indices[]; 
};

// 从打包的ubyte数组中提取ubyte值
uint extractUByte(uint packedValue, uint indexInUint) {
    uint shift = indexInUint * 8u;
    return (packedValue >> shift) & 0xFFu;
}

void main() {
    uint outIdx = gl_GlobalInvocationID.x;
    uint totalIndices = prefixSums[prefixSums.length() - 1u];
    
    if (outIdx >= totalIndices) {
        return;
    }
    
    // 二分查找批次
    int low = 0;
    int high = int(prefixSums.length() - 1u);
    
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (prefixSums[mid] > outIdx) {
            high = mid; // next [low, mid)
        } else {
            low = mid + 1; // next [mid + 1, high)
        }
    }
    
    uint batchID = uint(low);
    
    // 计算本地索引
    uint batchStart = (batchID == 0u) ? 0u : prefixSums[batchID - 1u];
    uint localIdx = outIdx - batchStart;
    
    // 读取原始索引（firstIndex是ubyte元素索引）
    uint inIndex = localIdx + firstIndex[batchID];
    
    // 计算在uint数组中的位置和偏移
    uint uintIndex = inIndex / 4u;
    uint offsetInUint = inIndex % 4u;
    
    uint packedValue = in_indices[uintIndex];
    uint originalIndex = extractUByte(packedValue, offsetInUint);
    
    // 应用基顶点偏移
    uint finalIndex = uint(int(originalIndex) + baseVertex[batchID]);
    
    // 写入输出
    out_indices[outIdx] = finalIndex;
}
)";

    // 类型到着色器的映射
    UnorderedMap<GLenum, GLuint> m_typeToProgram;
    
public:
    // 构造函数/析构函数
    MultiDrawElementsBaseVertex();
    ~MultiDrawElementsBaseVertex();
    
    // 禁止拷贝
    MultiDrawElementsBaseVertex(const MultiDrawElementsBaseVertex&) = delete;
    MultiDrawElementsBaseVertex& operator=(const MultiDrawElementsBaseVertex&) = delete;
    
    // 移动语义
    MultiDrawElementsBaseVertex(MultiDrawElementsBaseVertex&& other) noexcept;
    MultiDrawElementsBaseVertex& operator=(MultiDrawElementsBaseVertex&& other) noexcept;
    
    void multiDrawElementsBaseVertex(GLenum mode, 
                                    const GLsizei* count,
                                    GLenum type,
                                    const void* const* indices,
                                    GLsizei drawcount,
                                    const GLint* basevertex);
    
    // 清理和重置
    void clear();
    
private:
    // 内部辅助函数
    void initializeGPUResources();
    void compileComputePrograms();
    GLuint compileShader(GLenum type, const char* source);
    GLuint linkProgram(GLuint shader);
    void updatePrefixSums(const GLsizei* count, GLsizei drawcount);
    void uploadBatchDataToGPU(const GLsizei* count,
                             GLenum type,
                             const void* const* indices,
                             const GLint* basevertex,
                             GLsizei drawcount);
    void ensureCapacity();
    void runComputeShader(GLenum type, GLuint inputIBO, GLuint totalIndices);
    
    // 类型辅助函数
    GLuint getTypeSize(GLenum type) const;
};

// 全局实例
extern multiDrawElementsBaseVertex g_multiDrawElementsBaseVertex_shader;
