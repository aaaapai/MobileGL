//
// Created by BZLZHH on 2025/3/15.
//

#include "../../../../Includes.h"

namespace MG_GL::GL {
    Diligent::VALUE_TYPE ConvertGLIndexTypeToDiligent(GLenum type) {
        switch (type) {
            case GL_UNSIGNED_BYTE: return Diligent::VT_UINT8;
            case GL_UNSIGNED_SHORT: return Diligent::VT_UINT16;
            case GL_UNSIGNED_INT: return Diligent::VT_UINT32;
            default: return Diligent::VT_UNDEFINED;
        }
    }
    
    void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length,
                         GLbitfield access) {
        MG_Util::Debug::LogD("glMapBufferRange, target: %s, offset: %lld, length: %lld, access: 0x%X",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(length),
                             access);
        void* mappedPointer = nullptr;
        GLenum result = MG_State::AcquireBufferMemoryRange(target, offset, length, access, &mappedPointer);

        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Mapped pointer: %p", mappedPointer);
            return mappedPointer;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error mapping buffer range: %s", MG_Util::Debug::GLEnumToString(result));
        return nullptr;
    }

    void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
        MG_Util::Debug::LogD("glFlushMappedBufferRange, target: %s, offset: %lld, length: %lld",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(length));
        GLenum result = MG_State::SyncBufferMemory(target, offset, length);
        if (result == GL_NO_ERROR) {
            GLuint buffer = MG_State_T::bufferState->GetCurrentBinding(target);
            if (buffer == 0) return;

            auto& bufferObj = *MG_State_T::bufferState->GetBufferObject(buffer);
            if (!bufferObj.isMapped) return;

            size_t start = static_cast<size_t>(offset);
            size_t end = start + static_cast<size_t>(length);

            auto it = MG_Diligent::g_BufferMap.find(buffer);
            if (it == MG_Diligent::g_BufferMap.end())
                return;
            Diligent::IBuffer* pBuffer = it->second;
            if (pBuffer && bufferObj.data.size() >= end) {
                const auto& Desc = pBuffer->GetDesc();
                if (Desc.Usage == Diligent::USAGE_STAGING || Desc.Usage == Diligent::USAGE_DYNAMIC || Desc.Usage == Diligent::USAGE_UNIFIED) {
                    void * data;
                    MG_Diligent::g_pContext->MapBuffer(pBuffer, Diligent::MAP_WRITE,
                                                       Diligent::MAP_FLAG_DISCARD, data);

                    if (data) {
                        void* dst = static_cast<char*>(data) + offset;
                        const void* src = bufferObj.data.data() + offset;
                        memcpy(dst, src, length);

                        MG_Diligent::g_pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);
                    }
                } else {
                    // For USAGE_DEFAULT or USAGE_IMMUTABLE, use UpdateBuffer
                    MG_Diligent::g_pContext->UpdateBuffer(pBuffer, static_cast<Diligent::Uint64>(offset), static_cast<Diligent::Uint64>(length), bufferObj.data.data() + offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error flushing mapped buffer range: %s",
                             MG_Util::Debug::GLEnumToString(result));
    }

    void CopyBufferSubData(GLenum readTarget, GLenum writeTarget,
                           GLintptr readOffset, GLintptr writeOffset,
                           GLsizeiptr size) {
        MG_Util::Debug::LogD("glCopyBufferSubData, readTarget: %s, writeTarget: %s, "
                             "readOffset: %lld, writeOffset: %lld, size: %lld",
                             MG_Util::Debug::GLEnumToString(readTarget),
                             MG_Util::Debug::GLEnumToString(writeTarget),
                             static_cast<long long>(readOffset),
                             static_cast<long long>(writeOffset),
                             static_cast<long long>(size));
        GLenum result = MG_State::CopyBufferRange(readTarget, writeTarget, readOffset, writeOffset, size);
        if (result == GL_NO_ERROR) {
            GLuint srcBuffer = MG_State_T::bufferState->GetCurrentBinding(readTarget);
            GLuint dstBuffer = MG_State_T::bufferState->GetCurrentBinding(writeTarget);

            if (srcBuffer == 0 || dstBuffer == 0) return;
            auto itsrc = MG_Diligent::g_BufferMap.find(srcBuffer);
            auto itdst = MG_Diligent::g_BufferMap.find(dstBuffer);
            if (itsrc == MG_Diligent::g_BufferMap.end() || itdst == MG_Diligent::g_BufferMap.end()) return;

            Diligent::IBuffer* pSrcBuffer = itsrc->second;
            Diligent::IBuffer* pDstBuffer = itdst->second;

            if (pSrcBuffer && pDstBuffer) {
                MG_Diligent::g_pContext->CopyBuffer(pSrcBuffer, static_cast<Diligent::Uint64>(readOffset), 
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                    pDstBuffer, static_cast<Diligent::Uint64>(writeOffset),
                                                    static_cast<Diligent::Uint64>(size),
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
            
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error copying buffer subdata: %s",
                             MG_Util::Debug::GLEnumToString(result));
    }
    
    void* MapBuffer(GLenum target, GLenum access) {
        MG_Util::Debug::LogD("glMapBuffer(target=0x%x, access=0x%x)", target, access);
        void* mappedPtr = nullptr;
        GLenum err = MG_State::AcquireBufferMemory(target, access, &mappedPtr);
        if (err != GL_NO_ERROR) {
            MG_State::SetError(err);
            MG_Util::Debug::LogE("glMapBuffer failed: %s", MG_Util::Debug::GLEnumToString(err));
            return nullptr;
        }
        MG_Util::Debug::LogD("glMapBuffer returns %p", mappedPtr);
        return mappedPtr;
    }
    
    GLboolean UnmapBuffer(GLenum target) {
        MG_Util::Debug::LogD("glUnmapBuffer(target=0x%x)", target);
        GLenum err = MG_State::ReleaseBufferMemory(target);
        if (err != GL_NO_ERROR) {
            MG_State::SetError(err);
            MG_Util::Debug::LogE("glUnmapBuffer failed: %s", MG_Util::Debug::GLEnumToString(err));
            return GL_FALSE;
        }
        
        GLuint buffer = MG_State_T::bufferState->currentBindings_[target];
        auto& bufferObj = *MG_State_T::bufferState->GetBufferObject(buffer);

        auto it = MG_Diligent::g_BufferMap.find(buffer);
        if (it == MG_Diligent::g_BufferMap.end())
            return GL_FALSE;
        Diligent::IBuffer* pBuffer = it->second;
        if (pBuffer) {
            const auto& Desc = pBuffer->GetDesc();
            if (Desc.Usage == Diligent::USAGE_STAGING || Desc.Usage == Diligent::USAGE_DYNAMIC || Desc.Usage == Diligent::USAGE_UNIFIED) {
                void * data;
                MG_Diligent::g_pContext->MapBuffer(pBuffer, Diligent::MAP_WRITE,
                                                   Diligent::MAP_FLAG_DISCARD, data);

                if (data) {
                    memcpy(data, bufferObj.data.data(), bufferObj.data.size());
                    MG_Diligent::g_pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);
                }
            } else {
                // For USAGE_DEFAULT or USAGE_IMMUTABLE, use UpdateBuffer
                MG_Diligent::g_pContext->UpdateBuffer(pBuffer, 0, bufferObj.data.size(), bufferObj.data.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);    
            }
        }
        
        MG_Util::Debug::LogD("glUnmapBuffer succeeded");
        return GL_TRUE;
    }

    void BindBuffer(GLenum target, GLuint buffer) {
        MG_Util::Debug::LogD("glBindBuffer, target: %s, buffer: %d", MG_Util::Debug::GLEnumToString(target), buffer);
        if (buffer != 0 &&
            MG_State::ValidateGeneratedName(buffer) &&
            !MG_State::ValidateAllocatedBufferHandle(buffer)) {
            MG_Util::Debug::LogD("Actually creating buffer: %u", buffer);
            GLenum result = MG_State::CreateBuffer(buffer);
            if (result != GL_NO_ERROR) {
                MG_State::SetError(result);
                MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
                return;
            }
        }

        if (buffer != 0 && !MG_State::ValidateAllocatedBufferHandle(buffer)) {
            MG_State::SetError(GL_INVALID_VALUE);
            MG_Util::Debug::LogE("Invalid buffer handle: %u", buffer);
            return;
        }
        GLenum result = MG_State::BindBuffer(target, buffer);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        MG_Util::Debug::LogD("glBufferData, target: %s, size: %zd, data: %p, usage: %s",
                             MG_Util::Debug::GLEnumToString(target), size, data, MG_Util::Debug::GLEnumToString(usage));
        GLenum result = MG_State::CommitBufferStorage(target, size, data, usage);
        if (result == GL_NO_ERROR) {
            GLuint buffer = MG_State_T::bufferState->GetCurrentBinding(target);
            if (buffer == 0) return;
            auto& bufferObj = *MG_State_T::bufferState->GetBufferObject(buffer);
            if (bufferObj.isDynamic) return; // Dynamic buffer should be created by glDraw*
            if (size <= 0) return; // ignore 0-sized reallocation

            bufferObj.dirty = false;

            auto it = MG_Diligent::g_BufferMap.find(buffer);
            if (it == MG_Diligent::g_BufferMap.end())
                return;

            Diligent::IBuffer*& pBuffer = it->second;

            Diligent::BufferDesc BuffDesc;
            std::string name;
            BuffDesc.Size = static_cast<Diligent::Uint64>(size);

            switch (target) {
                case GL_ARRAY_BUFFER:
//                    BuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
                    name += std::format("VBO {}", buffer);
                    break;
                case GL_ELEMENT_ARRAY_BUFFER:
//                    BuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
                    name += std::format("IBO {}", buffer);
                    break;
                case GL_UNIFORM_BUFFER:
//                    BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
                    name += std::format("UBO {}", buffer);
                    break;
                default:
                    BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
                    name += std::format("SRV {}", buffer);
                    break;
            }

            BuffDesc.BindFlags |= Diligent::BIND_VERTEX_BUFFER |
                                  Diligent::BIND_INDEX_BUFFER |
                                  Diligent::BIND_UNIFORM_BUFFER;
            BuffDesc.Name = name.c_str();
            BuffDesc.Usage = Diligent::USAGE_DEFAULT;

            if (!pBuffer || (pBuffer && pBuffer->GetDesc().Size != bufferObj.data.size()) ) {
                if (pBuffer) {
                    pBuffer->Release();
                    pBuffer = nullptr;
                }
                Diligent::BufferData BuffData;
                // Initial data must not be null for immutable buffers
                BuffData.pData = BuffDesc.Usage != Diligent::USAGE_IMMUTABLE ? nullptr : bufferObj.data.data();
                BuffData.DataSize = static_cast<Diligent::Uint64>(size);
                MG_Diligent::g_pDevice->CreateBuffer(BuffDesc, &BuffData, &pBuffer);
            }
            
            if (data != nullptr && BuffDesc.Usage != Diligent::USAGE_IMMUTABLE) {
                    MG_Diligent::g_pContext->UpdateBuffer(pBuffer, 0,
                                                          static_cast<Diligent::Uint64>(size), data,
                                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
            return;
        }
        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params) {
        MG_Util::Debug::LogD("glGetBufferParameteriv, target: %d, pname: %d, params: %p",
                             target, pname, params);
        GLenum result = MG_State::QueryBufferPropertyIntVector(target, pname, params);
        if (result == GL_NO_ERROR)
            return;
        MG_State::SetError(result);
        MG_Util::Debug::LogE("GetBufferParameteriv not implemented");
    }
    
    void GenBuffers(GLsizei n, GLuint* buffers) {
        MG_Util::Debug::LogD("glGenBuffers, n: %d, buffers: %p", n, buffers);

        if (n < 0) {
            MG_State::SetError(GL_INVALID_VALUE);
            MG_Util::Debug::LogE("Invalid buffer count: %d", n);
            return;
        }

        GLenum result = MG_State::GenBufferNames(n, buffers);
        if (result == GL_NO_ERROR) {
            MG_Util::Debug::LogD("Generated buffer names:");
            for (GLsizei i = 0; i < n; ++i) {                 
                MG_Util::Debug::LogD("  Buffer[%d] = %u", i, buffers[i]);
                MG_Diligent::g_BufferMap[buffers[i]] = nullptr;
            }
            return;
        }

        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }

    GLboolean IsBuffer(GLuint buffer) {
        MG_Util::Debug::LogD("glIsBuffer, buffer: %u", buffer);

        if (buffer == 0) {
            return GL_FALSE;
        }

        bool isValid = MG_State::ValidateAllocatedBufferHandle(buffer);
        // Should we report gl error here or in MG_State? 
        MG_Util::Debug::LogD("Buffer %u is %s", buffer, isValid ? "valid" : "invalid");
        return isValid ? GL_TRUE : GL_FALSE;
    }

    void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
        MG_Util::Debug::LogD("glBufferSubData, target: %s, offset: %lld, size: %lld, data: %p",
                             MG_Util::Debug::GLEnumToString(target),
                             static_cast<long long>(offset),
                             static_cast<long long>(size),
                             data);
        GLenum result = MG_State::CommitBufferStorageRegion(target, offset, size, data);
        if (result != GL_NO_ERROR) {
            MG_State::SetError(result);
            MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
        }

        GLuint buffer = MG_State_T::bufferState->GetCurrentBinding(target);
        if (buffer == 0) return;

        auto& bufferObj = *MG_State_T::bufferState->GetBufferObject(buffer);
        if (bufferObj.isDynamic) return; // Dynamic buffer should be created by glDraw*

        auto it = MG_Diligent::g_BufferMap.find(buffer);
        if (it == MG_Diligent::g_BufferMap.end())
            return;
        Diligent::IBuffer* pBuffer = it->second;
        if (pBuffer) {
            const auto& Desc = pBuffer->GetDesc();
            if (Desc.Usage == Diligent::USAGE_STAGING || Desc.Usage == Diligent::USAGE_DYNAMIC || Desc.Usage == Diligent::USAGE_UNIFIED) {
                void* pMappedData = nullptr;
                MG_Diligent::g_pContext->MapBuffer(pBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, pMappedData);
                if (pMappedData)
                {
                    memcpy(static_cast<Diligent::Uint8*>(pMappedData) + offset, data, size);
                    MG_Diligent::g_pContext->UnmapBuffer(pBuffer, Diligent::MAP_WRITE);
                } else {
                    MG_Util::Debug::LogE("Failed to map buffer for BufferSubData");
                }
            } else {
                // For USAGE_DEFAULT or USAGE_IMMUTABLE, use UpdateBuffer
                MG_Diligent::g_pContext->UpdateBuffer(pBuffer, static_cast<Diligent::Uint64>(offset),
                                                      static_cast<Diligent::Uint64>(size), data,
                                                      Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
        }
    }
    void DeleteBuffers(GLsizei n, const GLuint *buffers) {
        MG_Util::Debug::LogD("glDeleteBuffers, n: %d, buffers: %p", n, buffers);

        GLenum result = MG_State::DeleteBuffers(n, buffers);
        if (result == GL_NO_ERROR) {
            for (GLsizei i = 0; i < n; ++i) {
                GLuint buffer = buffers[i];
                if (buffer != 0) {
                    auto it = MG_Diligent::g_BufferMap.find(buffer);
                    if (it != MG_Diligent::g_BufferMap.end()) {
                        if (it->second) {
                            it->second->Release();
                        }
                        MG_Diligent::g_BufferMap.erase(it);
                    }

                }
            }
            return;
        }

        MG_State::SetError(result);
        MG_Util::Debug::LogE("Error from MG State: %s", MG_Util::Debug::GLEnumToString(result));
    }
}