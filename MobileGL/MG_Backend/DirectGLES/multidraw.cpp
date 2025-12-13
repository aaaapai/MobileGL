#include "multidraw.h"
#include "DirectGLES.h"
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

// 全局实例
MultiDrawElementsBaseVertex g_multiDrawElementsBaseVertex_shader;

MultiDrawElementsBaseVertex::MultiDrawElementsBaseVertex() {
    m_prefixSums.reserve(128);
}

MultiDrawElementsBaseVertex::~MultiDrawElementsBaseVertex() {
    clear();
    
    if (m_initialized) {
        GLuint buffers[] = {m_prefixSumBuffer, m_firstIndexBuffer, 
                           m_baseVertexBuffer, m_outputIBO};
        MG_External::GLES::glDeleteBuffers(4, buffers);
        
        for (auto& pair : m_typeToProgram) {
            if (pair.second) {
                MG_External::GLES::glDeleteProgram(pair.second);
            }
        }
        m_typeToProgram.clear();
    }
}

MultiDrawElementsBaseVertex::MultiDrawElementsBaseVertex(MultiDrawElementsBaseVertex&& other) noexcept
    : m_prefixSumBuffer(std::exchange(other.m_prefixSumBuffer, 0))
    , m_firstIndexBuffer(std::exchange(other.m_firstIndexBuffer, 0))
    , m_baseVertexBuffer(std::exchange(other.m_baseVertexBuffer, 0))
    , m_outputIBO(std::exchange(other.m_outputIBO, 0))
    , m_typeToProgram(std::move(other.m_typeToProgram))
    , m_prefixSums(std::move(other.m_prefixSums))
    , m_totalIndices(std::exchange(other.m_totalIndices, 0))
    , m_initialized(std::exchange(other.m_initialized, false))
    , m_dataDirty(std::exchange(other.m_dataDirty, true)) {}

MultiDrawElementsBaseVertex& MultiDrawElementsBaseVertex::operator=(MultiDrawElementsBaseVertex&& other) noexcept {
    if (this != &other) {
        clear();
        
        if (m_initialized) {
            GLuint buffers[] = {m_prefixSumBuffer, m_firstIndexBuffer, 
                               m_baseVertexBuffer, m_outputIBO};
            MG_External::GLES::glDeleteBuffers(4, buffers);
            
            for (auto& pair : m_typeToProgram) {
                if (pair.second) {
                    MG_External::GLES::glDeleteProgram(pair.second);
                }
            }
            m_typeToProgram.clear();
        }
        
        m_prefixSumBuffer = std::exchange(other.m_prefixSumBuffer, 0);
        m_firstIndexBuffer = std::exchange(other.m_firstIndexBuffer, 0);
        m_baseVertexBuffer = std::exchange(other.m_baseVertexBuffer, 0);
        m_outputIBO = std::exchange(other.m_outputIBO, 0);
        m_typeToProgram = std::move(other.m_typeToProgram);
        m_prefixSums = std::move(other.m_prefixSums);
        m_totalIndices = std::exchange(other.m_totalIndices, 0);
        m_initialized = std::exchange(other.m_initialized, false);
        m_dataDirty = std::exchange(other.m_dataDirty, true);
    }
    return *this;
}

void MultiDrawElementsBaseVertex::multiDrawElementsBaseVertex(GLenum mode, 
                                                             const GLsizei* count,
                                                             GLenum type,
                                                             const void* const* indices,
                                                             GLsizei drawcount,
                                                             const GLint* basevertex) {
    
    // 首次使用则初始化GPU资源
    if (!m_initialized) {
        initializeGPUResources();
        m_initialized = true;
    }
    
    // 更新前缀和
    updatePrefixSums(count, drawcount);
    
    // 上传批次数据到GPU
    uploadBatchDataToGPU(count, type, indices, basevertex, drawcount);
    
    // 获取当前绑定的IBO
    GLint currentIBO = 0;
    MG_External::GLES::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentIBO);
    
    if (currentIBO == 0) {
        std::cerr << "Error: No IBO bound to current context.\n";
        return;
    }
    
    // 保存当前状态
    GLint prevProgram = 0;
    GLint prevArrayBuffer = 0;
    GLint prevElementArrayBuffer = 0;
    
    MG_External::GLES::glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    MG_External::GLES::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);
    MG_External::GLES::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &prevElementArrayBuffer);
    
    // 确保输出缓冲区足够大
    ensureCapacity();
    
    // 运行计算着色器处理索引
    runComputeShader(type, static_cast<GLuint>(currentIBO), m_totalIndices);
    
    // 绑定输出IBO进行绘制
    MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_outputIBO);
    MG_External::GLES::glDrawElements(mode, static_cast<GLsizei>(m_totalIndices), GL_UNSIGNED_INT, 0);
    
    // 恢复状态
    MG_External::GLES::glUseProgram(static_cast<GLuint>(prevProgram));
    MG_External::GLES::glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(prevArrayBuffer));
    MG_External::GLES::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(prevElementArrayBuffer));
}

void MultiDrawElementsBaseVertex::clear() {
    m_prefixSums.clear();
    m_totalIndices = 0;
    m_dataDirty = true;
}

void MultiDrawElementsBaseVertex::initializeGPUResources() {
    MG_External::GLES::glGenBuffers(1, &m_prefixSumBuffer);
    MG_External::GLES::glGenBuffers(1, &m_firstIndexBuffer);
    MG_External::GLES::glGenBuffers(1, &m_baseVertexBuffer);
    MG_External::GLES::glGenBuffers(1, &m_outputIBO);
    
    compileComputePrograms();
}

void MultiDrawElementsBaseVertex::compileComputePrograms() {
    // 编译GL_UNSIGNED_INT类型的着色器程序
    GLuint uintShader = compileShader(GL_COMPUTE_SHADER, COMPUTE_SHADER_SOURCE_UINT.data());
    GLuint uintProgram = linkProgram(uintShader);
    if (uintProgram) {
        m_typeToProgram[GL_UNSIGNED_INT] = uintProgram;
    }
    
    // 编译GL_UNSIGNED_SHORT类型的着色器程序
    GLuint ushortShader = compileShader(GL_COMPUTE_SHADER, COMPUTE_SHADER_SOURCE_USHORT.data());
    GLuint ushortProgram = linkProgram(ushortShader);
    if (ushortProgram) {
        m_typeToProgram[GL_UNSIGNED_SHORT] = ushortProgram;
    }
    
    // 编译GL_UNSIGNED_BYTE类型的着色器程序
    GLuint ubyteShader = compileShader(GL_COMPUTE_SHADER, COMPUTE_SHADER_SOURCE_UBYTE.data());
    GLuint ubyteProgram = linkProgram(ubyteShader);
    if (ubyteProgram) {
        m_typeToProgram[GL_UNSIGNED_BYTE] = ubyteProgram;
    }
    
    // 检查是否至少有一个程序编译成功
    if (m_typeToProgram.empty()) {
        std::cerr << "Error: Failed to compile any compute shader programs.\n";
    }
}

GLuint MultiDrawElementsBaseVertex::compileShader(GLenum type, const char* source) {
    GLuint shader = MG_External::GLES::glCreateShader(type);
    
    MG_External::GLES::glShaderSource(shader, 1, &source, nullptr);
    MG_External::GLES::glCompileShader(shader);
    
    // 检查编译状态
    GLint compileStatus = 0;
    MG_External::GLES::glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    
    if (compileStatus != GL_TRUE) {
        GLchar infoLog[1024];
        GLsizei logLength = 0;
        MG_External::GLES::glGetShaderInfoLog(shader, sizeof(infoLog), &logLength, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        MG_External::GLES::glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint MultiDrawElementsBaseVertex::linkProgram(GLuint shader) {
    GLuint program = MG_External::GLES::glCreateProgram();
    MG_External::GLES::glAttachShader(program, shader);
    MG_External::GLES::glLinkProgram(program);
    
    GLint linkStatus = 0;
    MG_External::GLES::glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    
    if (linkStatus != GL_TRUE) {
        GLchar infoLog[1024];
        GLsizei logLength = 0;
        MG_External::GLES::glGetProgramInfoLog(program, sizeof(infoLog), &logLength, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        MG_External::GLES::glDeleteProgram(program);
        program = 0;
    }
    
    MG_External::GLES::glDeleteShader(shader);
    return program;
}

void MultiDrawElementsBaseVertex::updatePrefixSums(const GLsizei* count, GLsizei drawcount) {
    if (drawcount <= 0) {
        m_prefixSums.clear();
        m_totalIndices = 0;
        return;
    }
    
    // 确保前缀和缓冲区足够大
    std::size_t requiredSize = static_cast<std::size_t>(drawcount);
    if (m_prefixSums.size() < requiredSize) {
        std::size_t newSize = m_prefixSums.capacity();
        while (newSize < requiredSize) {
            newSize = newSize == 0 ? 128 : newSize * 2;
        }
        m_prefixSums.reserve(newSize);
    }
    
    // 计算前缀和
    m_prefixSums.resize(requiredSize);
    m_totalIndices = 0;
    
    for (GLsizei i = 0; i < drawcount; ++i) {
        m_totalIndices += static_cast<GLuint>(count[i]);
        m_prefixSums[i] = m_totalIndices;
    }
}

void MultiDrawElementsBaseVertex::uploadBatchDataToGPU(const GLsizei* count,
                                                      GLenum type,
                                                      const void* const* indices,
                                                      const GLint* basevertex,
                                                      GLsizei drawcount) {
    if (drawcount <= 0) return;
    
    GLuint typeSize = getTypeSize(type);
    
    // 转换indices数组（字节偏移到元素索引）
    std::vector<GLuint> firstIndices(static_cast<std::size_t>(drawcount));
    for (GLsizei i = 0; i < drawcount; ++i) {
        // 将指针偏移转换为元素索引
        std::uintptr_t offset = reinterpret_cast<std::uintptr_t>(indices[i]);
        firstIndices[i] = static_cast<GLuint>(offset / typeSize);
    }
    
    // 上传firstIndex数据
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_firstIndexBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
                                    static_cast<GLsizeiptr>(firstIndices.size() * sizeof(GLuint)),
                                    firstIndices.data(), GL_DYNAMIC_DRAW);
    
    // 上传baseVertex数据
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_baseVertexBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER,
                                    static_cast<GLsizeiptr>(drawcount * sizeof(GLint)),
                                    basevertex, GL_DYNAMIC_DRAW);
    
    // 上传前缀和数据
    MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_prefixSumBuffer);
    MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER,
                                    static_cast<GLsizeiptr>(m_prefixSums.size() * sizeof(GLuint)),
                                    m_prefixSums.data(), GL_DYNAMIC_DRAW);
}

void MultiDrawElementsBaseVertex::ensureCapacity() {
    if (m_totalIndices > 0) {
        MG_External::GLES::glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_outputIBO);
        GLint currentSize = 0;
        MG_External::GLES::glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &currentSize);
        
        std::size_t requiredSize = m_totalIndices * sizeof(GLuint);
        if (static_cast<std::size_t>(currentSize) < requiredSize) {
            // 分配2的幂次方大小的缓冲区
            std::size_t newSize = 1024;
            while (newSize < requiredSize) {
                newSize *= 2;
            }
            MG_External::GLES::glBufferData(GL_SHADER_STORAGE_BUFFER, 
                                            static_cast<GLsizeiptr>(newSize), 
                                            nullptr, GL_DYNAMIC_DRAW);
        }
    }
}

void MultiDrawElementsBaseVertex::runComputeShader(GLenum type, GLuint inputIBO, GLuint totalIndices) {
    auto it = m_typeToProgram.find(type);
    if (it == m_typeToProgram.end() || it->second == 0 || totalIndices == 0) {
        std::cerr << "Error: No compute program available for type 0x" << std::hex << type << std::dec << "\n";
        return;
    }
    
    GLuint program = it->second;
    
    // 绑定输入IBO到计算着色器
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputIBO);       // 原始索引数据
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_firstIndexBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_baseVertexBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_prefixSumBuffer);
    MG_External::GLES::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_outputIBO);    // 输出索引数据
    
    // 使用计算着色器
    MG_External::GLES::glUseProgram(program);
    
    // 分发计算着色器
    const GLuint groupSize = 64;
    const GLuint numGroups = (totalIndices + groupSize - 1) / groupSize;
    
    MG_External::GLES::glDispatchCompute(numGroups, 1, 1);
    
    // 等待计算完成
    MG_External::GLES::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    // 恢复到默认程序
    MG_External::GLES::glUseProgram(0);
}

GLuint MultiDrawElementsBaseVertex::getTypeSize(GLenum type) const {
    switch (type) {
        case GL_UNSIGNED_BYTE:
            return 1;
        case GL_UNSIGNED_SHORT:
            return 2;
        case GL_UNSIGNED_INT:
            return 4;
        default:
            return 0;
    }
}
