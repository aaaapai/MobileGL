#include "DirectGLES.h"
#include "multidraw.h"
#include "Utils.h"
#include "Managers.h"
#include <MG_Util/Converters/GLToMG/TextureEnumConverter.h>
#include <MG_Util/Classifiers/TextureEnumClassifier.h>
#include <MG_Util/Metrics/TextureMetrics.h>
#include <MG_State/GLState/Core.h>
#include <MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h>
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/RenderStateEnumConverter.h>

namespace MobileGL::MG_Backend::DirectGLES {

    namespace {
        struct DrawElementsIndirectCommand {
            GLuint count;
            GLuint instanceCount;
            GLuint firstIndex;
            GLuint baseVertex;
            GLuint baseInstance;
        };
        
        thread_local struct {
            GLuint bufferId = 0;
            GLsizei capacity = 0;
            GLuint previousBinding = 0;
            bool bufferInitialized = false;
        } s_indirectBuffer;
        
        void InitializeIndirectBuffer() {
            if (!s_indirectBuffer.bufferInitialized) {
                MG_External::GLES::glGenBuffers(1, &s_indirectBuffer.bufferId);
                s_indirectBuffer.bufferInitialized = true;
                s_indirectBuffer.capacity = 0;
            }
        }
        
        void EnsureIndirectBufferCapacity(GLsizei requiredSize) {
            InitializeIndirectBuffer();
            
            if (s_indirectBuffer.capacity < requiredSize) {
                GLsizei newCapacity = s_indirectBuffer.capacity;
                if (newCapacity == 0) {
                    newCapacity = 16;
                }
                while (newCapacity < requiredSize) {
                    newCapacity *= 2;
                }
                
                MG_External::GLES::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, s_indirectBuffer.bufferId);
                MG_External::GLES::glBufferData(GL_DRAW_INDIRECT_BUFFER,
                    newCapacity * sizeof(DrawElementsIndirectCommand),
                    nullptr, GL_DYNAMIC_DRAW);
                
                s_indirectBuffer.capacity = newCapacity;
                MGLOG_D("Indirect buffer resized to capacity: %d commands", newCapacity);
            }
        }
        
        void SaveAndBindIndirectBuffer() {
            MG_External::GLES::glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, 
                reinterpret_cast<GLint*>(&s_indirectBuffer.previousBinding));
            MG_External::GLES::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, s_indirectBuffer.bufferId);
        }
        
        void RestoreIndirectBuffer() {
            MG_External::GLES::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, s_indirectBuffer.previousBinding);
        }
    }

    void MultiDrawElementsBaseVertex_indirect(GLenum mode, const GLsizei* count, GLenum type, 
                                     const GLvoid* const* indices, GLsizei drawcount, 
                                     const GLint* basevertex) {
#if MOBILEGL_LOG_ACTIVE_LEVEL <= MOBILEGL_LOG_LEVEL_DEBUG && MOBILEGL_ENABLE_SCOPE_MARKER
        DebugImpl::OpenGLScopeMarker marker(__func__);
#endif
        
        DrawSyncBit syncBit = DrawSyncBit::IndexBuffer | DrawSyncBit::IndirectBuffer;
        PrepareForDraw(syncBit);
        
        GLsizei elementSize = 4;
        switch (type) {
            case GL_UNSIGNED_BYTE:
                elementSize = 1;
                break;
            case GL_UNSIGNED_SHORT:
                elementSize = 2;
                break;
            case GL_UNSIGNED_INT:
                elementSize = 4;
                break;
            default:
                MGLOG_E("Unsupported index type: 0x%x", type);
                return;
        }
        
        Vector<DrawElementsIndirectCommand> commands(drawcount);
        
        for (GLsizei i = 0; i < drawcount; ++i) {
            commands[i].count = static_cast<GLuint>(count[i]);
            commands[i].instanceCount = 1;
            commands[i].baseVertex = basevertex ? static_cast<GLuint>(basevertex[i]) : 0;
            commands[i].baseInstance = 0;
            
            if (indices && indices[i]) {
                uintptr_t offset = reinterpret_cast<uintptr_t>(indices[i]);
                commands[i].firstIndex = static_cast<GLuint>(offset / elementSize);
            } else {
                commands[i].firstIndex = 0;
            }
        }
        
        EnsureIndirectBufferCapacity(drawcount);
        SaveAndBindIndirectBuffer();
        
        MG_External::GLES::glBufferSubData(GL_DRAW_INDIRECT_BUFFER,
            0, 
            drawcount * sizeof(DrawElementsIndirectCommand),
            commands.data());
        
        for (GLsizei i = 0; i < drawcount; ++i) {
            const void* offset = reinterpret_cast<const void*>(i * sizeof(DrawElementsIndirectCommand));
            MG_External::GLES::glDrawElementsIndirect(mode, type, offset);
        }
        
        RestoreIndirectBuffer();
    }
  
    class ComputeShaderManager {
private:
    UnorderedMap<GLenum, GLuint> m_programCache;
    
    const String m_shaderTemplateSource = R"(#version 310 es
layout(local_size_x = 64) in;

// 绘制命令数量
layout(std430, binding = 5) readonly buffer DrawCount { uint uDrawCount; };

#if ELEMENT_SIZE == 1
    layout(std430, binding = 0) readonly buffer Input { uint in_indices[]; };
#elif ELEMENT_SIZE == 2
    layout(std430, binding = 0) readonly buffer Input { uint in_indices[]; };
#else
    layout(std430, binding = 0) readonly buffer Input { uint in_indices[]; };
#endif

layout(std430, binding = 1) readonly buffer FirstIndex { uint firstIndex[]; };
layout(std430, binding = 2) readonly buffer BaseVertex { int baseVertex[]; };
layout(std430, binding = 3) readonly buffer Prefix { uint prefixSums[]; };
layout(std430, binding = 4) writeonly buffer Output { uint out_indices[]; };

void main() {
    uint outIdx = gl_GlobalInvocationID.x;
    uint drawCount = uDrawCount;
    
    // 检查是否超出总索引范围
    if (drawCount == 0u || outIdx >= prefixSums[drawCount - 1u]) {
        return;
    }
    
    // 二分查找
    uint low = 0u;
    uint high = drawCount;
    while (low < high) {
        uint mid = low + (high - low) / 2u;
        if (prefixSums[mid] > outIdx) {
            high = mid;
        } else {
            low = mid + 1u;
        }
    }
    
    uint cmdIndex = low;
    uint prevSum = (cmdIndex == 0u) ? 0u : prefixSums[cmdIndex - 1u];
    uint localIdx = outIdx - prevSum;
    uint inIndex = localIdx + firstIndex[cmdIndex];
    
    // 读取原始索引
    uint rawIndex = in_indices[inIndex];
    
    // 应用 baseVertex 偏移
    out_indices[outIdx] = uint(int(rawIndex) + baseVertex[cmdIndex]);
})";
    
public:
    ~ComputeShaderManager() {
        for (auto& pair : m_programCache) {
            if (pair.second != 0) {
                MG_External::GLES::glDeleteProgram(pair.second);
            }
        }
    }
    
    GLuint GetOrCreateProgram(GLenum type) {
        auto it = m_programCache.find(type);
        if (it != m_programCache.end()) {
            return it->second;
        }
        
        int elementSize = 4;
        switch (type) {
            case GL_UNSIGNED_BYTE: elementSize = 1; break;
            case GL_UNSIGNED_SHORT: elementSize = 2; break;
            case GL_UNSIGNED_INT: elementSize = 4; break;
            default:
                return 0;
        }
        
        String shaderSource = m_shaderTemplateSource;
        size_t pos = shaderSource.find("ELEMENT_SIZE");
        while (pos != String::npos) {
            shaderSource.replace(pos, 12, std::to_string(elementSize));
            pos = shaderSource.find("ELEMENT_SIZE", pos + 1);
        }
        
        GLuint program = MG_External::GLES::glCreateProgram();
        GLuint shader = MG_External::GLES::glCreateShader(GL_COMPUTE_SHADER);
        
        const char* sourceStr = shaderSource.c_str();
        MG_External::GLES::glShaderSource(shader, 1, &sourceStr, nullptr);
        MG_External::GLES::glCompileShader(shader);
        
        GLint success = 0;
        MG_External::GLES::glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            MG_External::GLES::glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            MG_External::GLES::glDeleteShader(shader);
            MG_External::GLES::glDeleteProgram(program);
            return 0;
        }
        
        MG_External::GLES::glAttachShader(program, shader);
        MG_External::GLES::glLinkProgram(program);
        MG_External::GLES::glDeleteShader(shader);
        
        MG_External::GLES::glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            MG_External::GLES::glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            MG_External::GLES::glDeleteProgram(program);
            return 0;
        }
        
        m_programCache[type] = program;
        return program;
    }
};

namespace {
    thread_local struct ComputeResources {
        GLuint prefixSumBuffer = 0;
        GLuint firstIndexBuffer = 0;
        GLuint baseVertexBuffer = 0;
        GLuint outputBuffer = 0;
        GLuint drawCountBuffer = 0;
        GLsizei outputBufferCapacity = 0;
        
        ComputeShaderManager shaderManager;
        
        ~ComputeResources() {
            if (prefixSumBuffer) MG_External::GLES::glDeleteBuffers(1, &prefixSumBuffer);
            if (firstIndexBuffer) MG_External::GLES::glDeleteBuffers(1, &firstIndexBuffer);
            if (baseVertexBuffer) MG_External::GLES::glDeleteBuffers(1, &baseVertexBuffer);
            if (outputBuffer) MG_External::GLES::glDeleteBuffers(1, &outputBuffer);
            if (drawCountBuffer) MG_External::GLES::glDeleteBuffers(1, &drawCountBuffer);
        }
        
        void EnsureInitialized() {
            if (!prefixSumBuffer) MG_External::GLES::glGenBuffers(1, &prefixSumBuffer);
            if (!firstIndexBuffer) MG_External::GLES::glGenBuffers(1, &firstIndexBuffer);
            if (!baseVertexBuffer) MG_External::GLES::glGenBuffers(1, &baseVertexBuffer);
            if (!outputBuffer) MG_External::GLES::glGenBuffers(1, &outputBuffer);
            if (!drawCountBuffer) MG_External::GLES::glGenBuffers(1, &drawCountBuffer);
        }
        
        void EnsureOutputBufferCapacity(GLsizei requiredIndices) {
            if (outputBufferCapacity < requiredIndices) {
                GLsizei newCapacity = outputBufferCapacity == 0 ? 1024 : outputBufferCapacity;
                while (newCapacity < requiredIndices) newCapacity *= 2;
                
                MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
                MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
                    newCapacity * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
                outputBufferCapacity = newCapacity;
            }
        }
    } s_computeResources;
}

void MultiDrawElementsBaseVertex_compute(
    GLenum mode,
    const GLsizei *count,
    GLenum type,
    const void * const *indices,
    GLsizei drawcount,
    const GLint *basevertex) {
    
    if (drawcount <= 0) return;
    if (!count || !indices) return;
    
    s_computeResources.EnsureInitialized();
    
    Vector<GLuint> prefixSums(drawcount);
    GLuint totalIndices = 0;
    for (GLsizei i = 0; i < drawcount; ++i) {
        totalIndices += static_cast<GLuint>(count[i]);
        prefixSums[i] = totalIndices;
    }
    
    if (totalIndices == 0) return;
    
    int elementSize = 4;
    switch (type) {
        case GL_UNSIGNED_BYTE: elementSize = 1; break;
        case GL_UNSIGNED_SHORT: elementSize = 2; break;
        case GL_UNSIGNED_INT: elementSize = 4; break;
        default: return;
    }
    
    Vector<GLuint> firstIndexInElements(drawcount);
    for (GLsizei i = 0; i < drawcount; ++i) {
        uintptr_t byteOffset = reinterpret_cast<uintptr_t>(indices[i]);
        firstIndexInElements[i] = static_cast<GLuint>(byteOffset / elementSize);
    }
    
    s_computeResources.EnsureOutputBufferCapacity(totalIndices);
    
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_computeResources.prefixSumBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
        drawcount * sizeof(GLuint), prefixSums.data(), GL_DYNAMIC_DRAW);
    
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_computeResources.firstIndexBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
        drawcount * sizeof(GLuint), firstIndexInElements.data(), GL_DYNAMIC_DRAW);
    
    if (basevertex) {
        MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_computeResources.baseVertexBuffer);
        MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
            drawcount * sizeof(GLint), basevertex, GL_DYNAMIC_DRAW);
    } else {
        Vector<GLint> zeroBaseVertex(drawcount, 0);
        MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_computeResources.baseVertexBuffer);
        MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
            drawcount * sizeof(GLint), zeroBaseVertex.data(), GL_DYNAMIC_DRAW);
    }
    
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_computeResources.drawCountBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
        sizeof(GLuint), &drawcount, GL_DYNAMIC_DRAW);
    
    GLint prevProgram = 0;
    GLint prevElementArrayBuffer = 0;
    MG_External::GLES::glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    MG_External::GLES::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &prevElementArrayBuffer);
    
    struct SSBOBinding { GLuint binding; GLuint buffer; };
    SSBOBinding prevSSBOBindings[6] = {};
    for (int i = 0; i < 6; ++i) {
        GLint buffer = 0;
        MG_External::GLES::glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, i, &buffer);
        prevSSBOBindings[i] = {static_cast<GLuint>(i), static_cast<GLuint>(buffer)};
    }
    
    if (prevElementArrayBuffer == 0) {
        MG_External::GLES::glUseProgram(prevProgram);
        MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prevElementArrayBuffer);
        return;
    }
    GLuint currentIBO = static_cast<GLuint>(prevElementArrayBuffer);
    
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, currentIBO);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, s_computeResources.firstIndexBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, s_computeResources.baseVertexBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, s_computeResources.prefixSumBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, s_computeResources.outputBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, s_computeResources.drawCountBuffer);
    
    GLuint computeProgram = s_computeResources.shaderManager.GetOrCreateProgram(type);
    if (computeProgram == 0) {
        MG_External::GLES::glUseProgram(prevProgram);
        MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prevElementArrayBuffer);
        for (const auto& binding : prevSSBOBindings) {
            MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding.binding, binding.buffer);
        }
        return;
    }
    
    MG_External::GLES::glUseProgram(computeProgram);
    
    MG_External::GLES::glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    
    GLuint workGroupCount = (totalIndices + 63) / 64;
    
    GLint maxWorkGroups[3] = {};
    MG_External::GLES::glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroups[0]);
    
    if (workGroupCount > static_cast<GLuint>(maxWorkGroups[0])) {
        GLuint batchSize = maxWorkGroups[0];
        GLuint processed = 0;
        GLuint remaining = totalIndices;
        
        while (remaining > 0) {
            GLuint batchGroups = (remaining > batchSize * 64) ? batchSize : (remaining + 63) / 64;
            
            GLint startOffsetLoc = MG_External::GLES::glGetUniformLocation(computeProgram, "uStartOffset");
            if (startOffsetLoc != -1) {
                MG_External::GLES::glUniform1ui(startOffsetLoc, processed);
            }
            
            MG_External::GLES::glDispatchCompute(batchGroups, 1, 1);
            MG_External::GLES::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            
            GLuint batchProcessed = batchGroups * 64;
            processed += batchProcessed;
            remaining = (remaining > batchProcessed) ? remaining - batchProcessed : 0;
        }
    } else {
        MG_External::GLES::glDispatchCompute(workGroupCount, 1, 1);
    }
    
    MG_External::GLES::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);
    
    MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_computeResources.outputBuffer);
    MG_External::GLES::glDrawElements(mode, totalIndices, GL_UNSIGNED_INT, 0);
    
    MG_External::GLES::glUseProgram(prevProgram);
    MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prevElementArrayBuffer);
    
    for (const auto& binding : prevSSBOBindings) {
        MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding.binding, binding.buffer);
    }
}

void MultiDrawElements_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
     MultiDrawElementsBaseVertex_indirect(mode, count, type, indices, drawcount, nullptr);
}

void MultiDrawElements_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
     MultiDrawElementsBaseVertex_compute(mode, count, type, indices, drawcount, nullptr);
}

}

MOBILEGL_GL_API void glMultiDrawElements_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount) {
      MobileGL::MG_Backend::DirectGLES::MultiDrawElements_indirect(mode, count, type, indices, drawcount);
}

MOBILEGL_GL_API void glMultiDrawElements_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount) {
      MobileGL::MG_Backend::DirectGLES::MultiDrawElements_compute(mode, count, type, indices, drawcount);
}

MOBILEGL_GL_API void glMultiDrawElementsBaseVertex_indirect(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex) {
      MobileGL::MG_Backend::DirectGLES::MultiDrawElementsBaseVertex_indirect(mode, count, type, indices, drawcount, basevertex);
}

MOBILEGL_GL_API void glMultiDrawElementsBaseVertex_compute(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex) {
      MobileGL::MG_Backend::DirectGLES::MultiDrawElementsBaseVertex_compute(mode, count, type, indices, drawcount, basevertex);
}
