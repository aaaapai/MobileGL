#include "Managers.h"
#include "MG_Backend/Backends.h"
#include "Utils.h"
#include "DirectGLES.h"
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToStr/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/DataTypeConverter.h>
#include <MG_Util/Converters/MGToGL/BufferEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
#include <MG_Util/Converters/MGToGL/ProgramEnumConverter.h>
#include <MG_Util/Converters/GLToMG/FramebufferEnumConverter.h>
#include <MG_Util/Converters/MGToGL/FramebufferEnumConverter.h>
#include <MG_State/GLState/FramebufferState/FramebufferObject.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace BufferImpl {
        BackendBufferObject::BackendBufferObject() {
            MG_External::GLES::glGenBuffers(1, &m_backendBufferId);
            if (m_backendBufferId == 0) {
                MGLOG_E("Failed to generate buffer object.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());
            } else {
                MGLOG_D("Generated buffer object with ID: %u.", m_backendBufferId);
            }
        }

        const GLenum TempBufferTarget = GL_ARRAY_BUFFER;
        void BackendBufferObject::SyncToBackend(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject) {
            if (!stateBufferObject) {
                MGLOG_E("State buffer object is null, cannot sync to backend.");
                return;
            }

            SizeT bufferSize = stateBufferObject->GetSize();
            if (bufferSize == 0) {
                MGLOG_W("Buffer size is zero, skipping sync for object with ID: %u", m_backendBufferId);
                return;
            }

            MGLOG_D("Syncing buffer object with backend ID %u to backend for state ID %u", m_backendBufferId,
                    stateBufferObject->GetExternalIndex());

            Bool needsRegeneration =
                !m_isInitialized || bufferSize > m_prevBufferSize || bufferSize < m_prevBufferSize / 2;

            if (needsRegeneration) {
                MGLOG_D("Buffer size changed significantly or not initialized, regenerating buffer with ID: %u",
                        m_backendBufferId);
                SyncToBackend_glBufferData(stateBufferObject);
                m_isInitialized = true;
                m_prevBufferSize = bufferSize;
                return;
            }

            switch (stateBufferObject->GetUsage()) {
            case BufferUsage::StaticDraw:
                SyncToBackend_glBufferSubData(stateBufferObject);
                break;
            case BufferUsage::DynamicDraw:
            case BufferUsage::StreamDraw:
                SyncToBackend_glMapBufferRange(stateBufferObject);
                break;
            default:
                SyncToBackend_glBufferSubData(stateBufferObject);
                break;
            }

            stateBufferObject->ClearDirty();
            m_prevBufferSize = bufferSize;
        }

        void BackendBufferObject::SyncToBackend_glBufferData(
            SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject) {
            BackendBufferBindingProtector backendBufferBindingProtector(TempBufferTarget);

            MGLOG_D("Syncing buffer data (glBufferData) for object with ID : %u", m_backendBufferId);

            const void* data = stateBufferObject->GetDataReadOnly()->data();
            SizeT size = stateBufferObject->GetSize();
            GLenum usage = MG_Util::ConvertBufferUsageToGLEnum(stateBufferObject->GetUsage());

            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
            MG_External::GLES::glBufferData(TempBufferTarget, size, data, usage);

            stateBufferObject->ClearDirty();
        }

        void BackendBufferObject::SyncToBackend_glBufferSubData(
            SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject) {
            BackendBufferBindingProtector backendBufferBindingProtector(TempBufferTarget);

            MGLOG_D("Syncing buffer sub-data (glBufferSubData) for object with ID : %u", m_backendBufferId);

            const void* data = stateBufferObject->GetDataReadOnly()->data();
            // dirty range: [range.start, range.end)
            const auto& range = stateBufferObject->GetDirtyRange();
            if (range.end == 0) {
                MGLOG_D("No dirty range to sync for buffer with ID: %u", m_backendBufferId);
                return;
            }

            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
            MG_External::GLES::glBufferSubData(TempBufferTarget, range.start, range.end - range.start,
                                               reinterpret_cast<const char*>(data) + range.start);
        }

        void BackendBufferObject::SyncToBackend_glMapBufferRange(
            SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject, Bool invalidate) {
            BackendBufferBindingProtector backendBufferBindingProtector(TempBufferTarget);

            MGLOG_D("Syncing buffer map (glMapBuffer) for object with ID : %u", m_backendBufferId);
            MGLOG_D("Mapping buffer with ID: %u", m_backendBufferId);
            const auto& range = stateBufferObject->GetDirtyRange();
            if (range.end == 0) {
                MGLOG_D("No dirty range to sync for buffer with ID: %u", m_backendBufferId);
                return;
            }
            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
            void* mappedData =
                MG_External::GLES::glMapBufferRange(TempBufferTarget, range.start, range.end - range.start,
                                                    (invalidate ? GL_MAP_INVALIDATE_BUFFER_BIT : 0) | GL_MAP_WRITE_BIT);
            const void* data = stateBufferObject->GetDataReadOnly()->data();
            if (mappedData) {
                Memcpy(mappedData, ((const char*)(data) + range.start), range.end - range.start);
                MGLOG_D("Mapped buffer data successfully for object with ID: %u", m_backendBufferId);
                MG_External::GLES::glUnmapBuffer(TempBufferTarget);
            } else {
                MGLOG_E("Failed to map buffer with ID: %u", m_backendBufferId);
            }
        }

        void BackendBufferObject::Bind() {
            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
        }

        void BackendBufferObject::Bind(GLenum target) {
            MG_External::GLES::glBindBuffer(target, m_backendBufferId);
        }

        UnorderedMap<SharedPtr<MG_State::GLState::BufferObject>, SharedPtr<BackendBufferObject>> g_backendBufferObjects;
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        BackendVertexArrayObject::BackendVertexArrayObject() {
            MG_External::GLES::glGenVertexArrays(1, &m_backendVAOId);
            if (m_backendVAOId == 0) {
                MGLOG_E("Failed to generate vertex array object.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());
            } else {
                MGLOG_D("Generated vertex array object with ID: %u.", m_backendVAOId);
            }
        }

        void BackendVertexArrayObject::Bind() {
            MG_External::GLES::glBindVertexArray(m_backendVAOId);
        }

        void BackendVertexArrayObject::SyncToBackend(SharedPtr<MG_State::GLState::VertexArrayObject>& stateVAOObject,
                                                     Bool needDivisor) {
            if (!stateVAOObject) {
                MGLOG_E("State VAO object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing VAO with backend ID %u to backend for state ID %u", m_backendVAOId,
                    stateVAOObject->GetExternalIndex());

            BufferImpl::BackendBufferBindingProtector backendBufferBindingProtector(BufferImpl::TempBufferTarget);
            BackendVertexArrayBindingProtector backendVAOBindingProtector;

            Bind();

            for (const auto& attribIndex : stateVAOObject->GetDirtyAttributeIndices()) {
                const auto& attrib = stateVAOObject->GetAttribute(attribIndex);
                if (attrib.Enabled) {
                    MGLOG_D("Binding attribute index %u for VAO ID: %u", attribIndex, m_backendVAOId);
                    MG_External::GLES::glEnableVertexAttribArray(attribIndex);
                } else {
                    MGLOG_D("Disabling attribute index %u for VAO ID: %u", attribIndex, m_backendVAOId);
                    MG_External::GLES::glDisableVertexAttribArray(attribIndex);
                    continue;
                }

                const auto& bufferObject = attrib.Buffer;
                if (!bufferObject) {
                    MGLOG_W("Attribute has no bound buffer, skipping.");
                    continue;
                }

                const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(bufferObject);
                if (backendBufferIt == BufferImpl::g_backendBufferObjects.end()) {
                    MGLOG_E("No backend buffer found for attribute's buffer, cannot bind attribute.");
                    continue;
                }
                const auto& backendBufferObject = backendBufferIt->second;

                backendBufferObject->Bind(GL_ARRAY_BUFFER);
                if (!attrib.IsInteger) {
                    MG_External::GLES::glVertexAttribPointer(
                        attribIndex, attrib.Size, MG_Util::ConvertDataTypeToGLEnum(attrib.Type),
                        attrib.Normalized ? GL_TRUE : GL_FALSE, attrib.Stride, (const void*)attrib.Offset);
                } else {
                    MG_External::GLES::glVertexAttribIPointer(attribIndex, attrib.Size,
                                                              MG_Util::ConvertDataTypeToGLEnum(attrib.Type),
                                                              attrib.Stride, (const void*)attrib.Offset);
                }

                if (needDivisor) {
                    MG_External::GLES::glVertexAttribDivisor(attribIndex, attrib.Divisor);
                }
            }

            const auto& indexBufferBinding = stateVAOObject->GetIndexBufferBindingSlot().GetBoundObject();
            if (indexBufferBinding) {
                const auto& backendBufferIt = BufferImpl::g_backendBufferObjects.find(indexBufferBinding);
                if (backendBufferIt != BufferImpl::g_backendBufferObjects.end()) {
                    const auto& backendBufferObject = backendBufferIt->second;
                    backendBufferObject->Bind(GL_ELEMENT_ARRAY_BUFFER);
                } else {
                    MGLOG_E("No backend buffer found for index buffer binding, cannot bind index buffer.");
                }
            }

            stateVAOObject->ClearDirtyAttributes();
        }

        UnorderedMap<SharedPtr<MG_State::GLState::VertexArrayObject>, SharedPtr<BackendVertexArrayObject>>
            g_backendVertexArrayObjects;
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        BackendTextureObject::BackendTextureObject() {
            MG_External::GLES::glGenTextures(1, &m_backendTextureId);
            if (m_backendTextureId == 0) {
                MGLOG_E("Failed to generate texture object.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());
            } else {
                MGLOG_D("Generated texture object with ID: %u.", m_backendTextureId);
            }
        }

        void BackendTextureObject::Bind(GLenum target) {
            MG_External::GLES::glBindTexture(target, m_backendTextureId);
        }

        Uint BackendTextureObject::GetBackendTextureId() {
            return m_backendTextureId;
        }

        void BackendTextureObject::SyncToBackend(SharedPtr<MG_State::GLState::ITextureObject>& stateTextureObject) {
            DebugImpl::ErrorLopper errorLopper;
            if (!stateTextureObject) {
                MGLOG_E("State texture object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing texture with backend ID %u to backend for state ID %u", m_backendTextureId,
                    stateTextureObject->GetExternalIndex());

            GLenum target = MG_Util::ConvertTextureTargetToGLEnum(stateTextureObject->GetTarget());
            auto targetInternal = stateTextureObject->GetTarget();
            MGLOG_D("    Texture target for syncing is %s",
                    MG_Util::ConvertTextureTargetToString(targetInternal).c_str());
            if (targetInternal == TextureTarget::TextureBuffer || targetInternal == TextureTarget::Texture1D ||
                targetInternal == TextureTarget::TextureRectangle ||
                targetInternal == TextureTarget::Texture2DMultisampleArray ||
                targetInternal == TextureTarget::Texture1DArray || targetInternal == TextureTarget::Texture3D ||
                targetInternal == TextureTarget::Texture2DMultisample ||
                targetInternal == TextureTarget::Texture2DArray) {
                MGLOG_D("    Texture target %s is not supported, skipping.",
                        MG_Util::ConvertTextureTargetToString(targetInternal).c_str());
                return;
            }

            // The texture needs to be regenerated completely with glTexImage* calls if:
            // 1. Not initialized
            // 2. InternalFormat changed
            // 3. Size changed
            // 4. Mipmap levels changed

            if (!stateTextureObject->IsComplete()) {
                MGLOG_D("Texture object with ID: %u is not complete, skipping sync.", m_backendTextureId);
                return;
            }

            BackendTextureBindingProtector backendTextureBindingProtector(target);
            Bind(target);
            errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                MGLOG_D("%s(%s:%d) ES error: %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });
            const auto baseSize = stateTextureObject->GetBaseSize();
            StateTextureBasicInfo currentTextureInfo = {
                stateTextureObject->GetFormat(), static_cast<SizeT>(baseSize.x()), static_cast<SizeT>(baseSize.y()),
                static_cast<SizeT>(baseSize.z()), 0};
            switch (stateTextureObject->GetStorageType()) {
                case TextureStorageType::Mipmap: {
                    auto* textureMipmapObject = static_cast<MG_State::GLState::TextureObjectMipmap*>(stateTextureObject.get());
                    const auto mipmapCount = textureMipmapObject->GetMipmapLevelCount();
                    currentTextureInfo.mipmapLevels = mipmapCount;

                    Bool needsRegeneration = !m_isInitialized || (currentTextureInfo != m_prevTextureInfo);

                    MGLOG_D("%s: Got texture info: %dx%dx%d, mips %d, format %s", __func__, baseSize.x(), baseSize.y(),
                            baseSize.z(), mipmapCount,
                            MG_Util::ConvertTextureInternalFormatToString(textureMipmapObject->GetFormat()).c_str());

                    if (needsRegeneration) {
                        MGLOG_D("Texture state changed significantly or not initialized, regenerating texture with ID: %u",
                                m_backendTextureId);

                        // Regenerate all mipmap levels
                        GLenum glInternalFormat, glType, glFormat;
                        TextureImpl::GenerateTextureFormatInfo(textureMipmapObject->GetFormat(), &glInternalFormat, &glType,
                                                               &glFormat);

                        const auto& uploadTargets = textureMipmapObject->GetUploadTargets();
                        for (auto uploadTarget : uploadTargets) {
                            for (SizeT level = 0; level < mipmapCount; ++level) {
                                auto levelTexelSize = textureMipmapObject->GetMipmapTexelSize(uploadTarget, level);
                                auto levelByteSize = textureMipmapObject->GetMipmapByteSize(uploadTarget, level);
                                bool levelDirty = textureMipmapObject->IsStorageDirty(uploadTarget, 0);
                                auto glUploadTarget = MG_Util::ConvertTextureUploadTargetToGLEnum(uploadTarget);
                                auto* pData = (levelDirty && levelByteSize != 0)
                                                  ? textureMipmapObject->MapMipmapData(uploadTarget, level)
                                                  : nullptr;
                                MGLOG_D("%s: target: %s: syncing mip %d: %dx%dx%d, byteSize = %d, pData = %p", __func__,
                                        MG_Util::ConvertTextureUploadTargetToString(uploadTarget).c_str(), level,
                                        levelTexelSize.x(), levelTexelSize.y(), levelTexelSize.z(), levelByteSize, pData);
                                BufferImpl::BackendBufferBindingProtector pixelUnpackProtector =
                                    BufferImpl::BackendBufferBindingProtector(GL_PIXEL_UNPACK_BUFFER);
                                errorLopper.Clear();
                                MG_External::GLES::glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                                MG_External::GLES::glTexImage2D(glUploadTarget, static_cast<GLint>(level), glInternalFormat,
                                                                static_cast<GLsizei>(levelTexelSize.x()),
                                                                static_cast<GLsizei>(levelTexelSize.y()), 0, glFormat, glType,
                                                                pData);

                                // TODO: handle more texture types

                                MGLOG_D("Regenerated mipmap level %d for texture with ID: %u", level, m_backendTextureId);
                                textureMipmapObject->MarkStorageDirty(uploadTarget, level, false);
                            }
                        }

                        m_isInitialized = true;
                    }

                    { // Update all dirty mipmap levels
                        const auto mipmapCount = textureMipmapObject->GetMipmapLevelCount();
                        GLenum glInternalFormat, glType, glFormat;
                        TextureImpl::GenerateTextureFormatInfo(textureMipmapObject->GetFormat(), &glInternalFormat, &glType,
                                                               &glFormat);
                        const auto& uploadTargets = textureMipmapObject->GetUploadTargets();
                        for (auto uploadTarget : uploadTargets) {
                            for (SizeT level = 0; level < mipmapCount; ++level) {
                                if (!textureMipmapObject->IsStorageDirty(uploadTarget, level)) {
                                    continue;
                                }

                                auto byteSize = textureMipmapObject->GetMipmapByteSize(uploadTarget, level);
                                if (byteSize == 0) {
                                    MGLOG_W("Mipmap level %d has no data, skipping update.", level);
                                    continue;
                                }

                                auto glUploadTarget = MG_Util::ConvertTextureUploadTargetToGLEnum(uploadTarget);

                                BufferImpl::BackendBufferBindingProtector pixelUnpackProtector =
                                    BufferImpl::BackendBufferBindingProtector(GL_PIXEL_UNPACK_BUFFER);
                                MG_External::GLES::glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                                errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                                    MGLOG_D("%s(%s:%d) ES error: %s", func, file, line,
                                            MG_Util::ConvertGLEnumToString(err).c_str());
                                });
                                auto texelSize = textureMipmapObject->GetMipmapTexelSize(uploadTarget, level);
                                MG_External::GLES::glTexSubImage2D(glUploadTarget, static_cast<GLint>(level), 0, 0,
                                                                   static_cast<GLsizei>(texelSize.x()),
                                                                   static_cast<GLsizei>(texelSize.y()), glFormat, glType,
                                                                   textureMipmapObject->MapMipmapData(uploadTarget, level));

                                textureMipmapObject->MarkStorageDirty(uploadTarget, level, false);
                            }
                        }
                    }
                    break;
                }
                default: THROW_UNIMPL_EXCEPTION;
            }

            { // Update built-in sampler parameters
                MGLOG_D("Updating sampler parameters for texture with ID: %u", m_backendTextureId);
                const auto& samplerParams = stateTextureObject->GetSamplerObject()->GetAllSamplerParameters();

#define SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(internalName, glName, type)                                                  \
    if (m_cacheSamplerParameters.internalName != samplerParams.internalName) {                                         \
        MG_External::GLES::glTexParameteri(target, glName,                                                             \
                                           MG_Util::ConvertSampler##type##ToGLEnum(samplerParams.internalName));       \
        m_cacheSamplerParameters.internalName = samplerParams.internalName;                                            \
        errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__,                                           \
                          t = MG_Util::ConvertSampler##type##ToGLEnum(samplerParams.internalName)](GLenum err) {       \
            MGLOG_D("%s(%s:%d) ES error %s, GL_TEXTURE_MIN_FILTER = %s", func, file, line,                             \
                    MG_Util::ConvertGLEnumToString(err).c_str(), MG_Util::ConvertGLEnumToString(t).c_str());           \
        });                                                                                                            \
    }

                if (m_cacheSamplerParameters.minFilter != samplerParams.minFilter ||
                    m_cacheSamplerParameters.mipmapMode != samplerParams.mipmapMode) {
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_MIN_FILTER,
                        MG_Util::ConvertSamplerFilterModeToGLEnum(samplerParams.minFilter, samplerParams.mipmapMode));
                    m_cacheSamplerParameters.minFilter = samplerParams.minFilter;
                    m_cacheSamplerParameters.mipmapMode = samplerParams.mipmapMode;
                }
                if (m_cacheSamplerParameters.magFilter != samplerParams.magFilter) {
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_MAG_FILTER,
                        MG_Util::ConvertSamplerFilterModeToGLEnum(samplerParams.magFilter, SamplerMipmapMode::None));
                    m_cacheSamplerParameters.magFilter = samplerParams.magFilter;
                }
                errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                    MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                });

                SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(wrapS, GL_TEXTURE_WRAP_S, WrapMode)
                SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(wrapT, GL_TEXTURE_WRAP_T, WrapMode)
                SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(wrapR, GL_TEXTURE_WRAP_R, WrapMode)
                SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(compareFunc, GL_TEXTURE_COMPARE_FUNC, CompareFunc)
                SYNC_TEX_SAMPLER_PARAM_IF_CHANGED(compareMode, GL_TEXTURE_COMPARE_MODE, CompareMode)
                if (m_cacheSamplerParameters.minLod != samplerParams.minLod) {
                    MG_External::GLES::glTexParameterf(target, GL_TEXTURE_MIN_LOD, samplerParams.minLod);
                    m_cacheSamplerParameters.minLod = samplerParams.minLod;
                }
                if (m_cacheSamplerParameters.maxLod != samplerParams.maxLod) {
                    MG_External::GLES::glTexParameterf(target, GL_TEXTURE_MAX_LOD, samplerParams.maxLod);
                    m_cacheSamplerParameters.maxLod = samplerParams.maxLod;
                }
                errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                    MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                });
#undef SYNC_TEX_SAMPLER_PARAM_IF_CHANGED
            }

            { // Update texture parameters
                MGLOG_D("Updating texture parameters for texture with ID: %u", m_backendTextureId);

                const auto& levelRange = stateTextureObject->GetLevelRange();

                if (m_cacheLodRange.x() != levelRange.x()) {
                    MG_External::GLES::glTexParameteri(target, GL_TEXTURE_BASE_LEVEL,
                                                       static_cast<GLint>(levelRange.x()));
                    m_cacheLodRange.x() = levelRange.x();
                }
                errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                    MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                });
                if (m_cacheLodRange.y() != levelRange.y()) {
                    MG_External::GLES::glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
                                                       static_cast<GLint>(levelRange.y()));
                    m_cacheLodRange.y() = levelRange.y();
                }
                errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                    MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                });

                const auto& swizzleParams = stateTextureObject->GetAllSwizzleParams();
                if (swizzleParams != m_cacheSwizzleParams) {
#define SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED(func, glEnum)                                                                \
    if (m_cacheSwizzleParams.func != swizzleParams.func) {                                                             \
        MG_External::GLES::glTexParameteri(target, glEnum,                                                             \
                                           MG_Util::ConvertTextureSwizzleParamToGLEnum(swizzleParams.func));           \
        m_cacheSwizzleParams.func = swizzleParams.func;                                                                \
    }
                    SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED(r(), GL_TEXTURE_SWIZZLE_R);
                    SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED(g(), GL_TEXTURE_SWIZZLE_G);
                    SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED(b(), GL_TEXTURE_SWIZZLE_B);
                    SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED(a(), GL_TEXTURE_SWIZZLE_A);
#undef SYNC_TEX_SWIZZLE_PARAM_IF_CHANGED
                    m_cacheSwizzleParams = swizzleParams;
                    errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                        MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                    });
                }

                if (m_cacheBorderColor != stateTextureObject->GetBorderColor()) {
                    const auto& borderColor = stateTextureObject->GetBorderColor();
                    GLfloat borderColorArray[4] = {borderColor.x(), borderColor.y(), borderColor.z(), borderColor.w()};
                    MG_External::GLES::glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColorArray);
                    m_cacheBorderColor = borderColor;
                    errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                        MGLOG_D("%s(%s:%d) ES error %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
                    });
                }
            }

            errorLopper.Loop([file = __FILE__, line = __LINE__, func = __func__](GLenum err) {
                MGLOG_D("%s(%s:%d) ES error: %s", func, file, line, MG_Util::ConvertGLEnumToString(err).c_str());
            });

            m_prevTextureInfo = currentTextureInfo;
        }

        UnorderedMap<SharedPtr<MG_State::GLState::ITextureObject>, SharedPtr<BackendTextureObject>>
            g_backendTextureObjects;
    } // namespace TextureImpl

    namespace FramebufferImpl {
        BackendFramebufferObject::BackendFramebufferObject() {
            MG_External::GLES::glGenFramebuffers(1, &m_backendFBOId);
            if (m_backendFBOId == 0) {
                MGLOG_E("Failed to generate framebuffer object.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());
            } else {
                MGLOG_D("Generated framebuffer object with ID: %u.", m_backendFBOId);
            }
        }

        void BackendFramebufferObject::Bind(FramebufferTarget target) {
            if (target == FramebufferTarget::Read)
                MG_External::GLES::glBindFramebuffer(GL_READ_FRAMEBUFFER, m_backendFBOId);
            else
                MG_External::GLES::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_backendFBOId);
        }

        void BackendFramebufferObject::SyncToBackend(SharedPtr<MG_State::GLState::FramebufferObject>& stateFBOObject,
                                                     FramebufferTarget asTarget) {
            if (!stateFBOObject) {
                MGLOG_E("State FBO object is null, cannot sync to backend.");
                return;
            }
            MGLOG_D("Syncing FBO with backend ID %u to backend for state ID %u, as %s FBO", m_backendFBOId,
                    stateFBOObject->GetExternalIndex(), (asTarget == FramebufferTarget::Draw ? "DRAW" : "READ"));
            GLenum glFBOTarget = MG_Util::ConvertFramebufferTargetToGLEnum(asTarget);
            BackendFramebufferBindingProtector backendFBOBindingProtector(glFBOTarget);
            Bind(asTarget);

            // Handle all attachments
            const auto& attachments = stateFBOObject->GetAllAttachments();
            for (SizeT i = 0; i < attachments.size(); ++i) {
                const auto& attachment = attachments[i];
                if (!attachment.IsValid() || attachment.IsEmpty()) {
                    continue;
                }
                FramebufferAttachmentType type = static_cast<FramebufferAttachmentType>(i);
                GLenum glAttachment = MG_Util::ConvertFramebufferAttachmentTypeToGLEnum(type);
                if (attachment.IsTexture()) {
                    const auto& textureObject = attachment.GetTexture();
                    const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                    if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
                        MGLOG_E("No backend texture found for FBO attachment, cannot bind texture.");
                        continue;
                    }
                    const auto& backendTextureObject = backendTextureIt->second;
                    auto glTextureTarget = MG_Util::ConvertTextureTargetToGLEnum(textureObject->GetTarget());
                    backendTextureObject->Bind(glTextureTarget);
                    MG_External::GLES::glFramebufferTexture2D(glFBOTarget, glAttachment, glTextureTarget,
                                                              backendTextureObject->GetBackendTextureId(),
                                                              static_cast<GLint>(attachment.GetTextureLevel()));
                } else if (attachment.IsRenderbuffer()) {
                    // TODO: renderbuffer support
                }
            }
            // Handle draw buffers for DRAW_FRAMEBUFFER
            if (asTarget == FramebufferTarget::Draw) {
                // Create mappings for draw buffers
                int nBuffers = 0;
                std::fill(m_frontendDrawBuffers,
                          m_frontendDrawBuffers + MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS,
                          FramebufferAttachmentType::None);
                std::fill(m_compactedFrontendDrawBuffers,
                          m_compactedFrontendDrawBuffers + MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS,
                          FramebufferAttachmentType::None);
                std::fill(m_backendDrawBuffers,
                          m_backendDrawBuffers + MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS, GL_NONE);
                auto& stateDrawBuffers = stateFBOObject->GetDrawBuffers();
                for (GLint i = 0; i < MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS; ++i) {
                    if (stateDrawBuffers[i] == FramebufferAttachmentType::None) {
                        m_frontendDrawBuffers[i] = FramebufferAttachmentType::None;
                        continue;
                    }

                    m_frontendDrawBuffers[i] = stateDrawBuffers[i];

                    // Create compacted mapping
                    m_backendDrawBuffers[nBuffers] = GL_COLOR_ATTACHMENT0 + nBuffers;
                    m_compactedFrontendDrawBuffers[nBuffers] = m_frontendDrawBuffers[i];
                    nBuffers++;
                }

                MG_External::GLES::glDrawBuffers(nBuffers, m_backendDrawBuffers);
                stateFBOObject->ClearDrawBuffersDirtyState();
            }
            // Handle read buffer for READ_FRAMEBUFFER
            else if (asTarget == FramebufferTarget::Read) {
                m_frontendReadBuffer = stateFBOObject->GetReadBuffer();
                GLenum frontendAtt = MG_Util::ConvertFramebufferAttachmentTypeToGLEnum(m_frontendReadBuffer);
                GLenum backendAtt = GL_NONE;
                const auto& readAttachment = attachments[(SizeT)m_frontendReadBuffer];
                GLenum glAttachment = MG_Util::ConvertFramebufferAttachmentTypeToGLEnum(m_frontendReadBuffer);
                if (!readAttachment.IsValid() || readAttachment.IsEmpty()) {
                    return;
                }
                if (readAttachment.IsTexture()) {
                    const auto& textureObject = readAttachment.GetTexture();
                    const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                    if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
                        MGLOG_E("ReadBuffer: No backend texture found for FBO attachment, cannot bind texture.");
                        return;
                    }
                    const auto& backendTextureObject = backendTextureIt->second;
                    auto glTextureTarget = MG_Util::ConvertTextureTargetToGLEnum(textureObject->GetTarget());
                    backendTextureObject->Bind(glTextureTarget);
                    MG_External::GLES::glFramebufferTexture2D(glFBOTarget, glAttachment, glTextureTarget,
                                                              backendTextureObject->GetBackendTextureId(),
                                                              static_cast<GLint>(readAttachment.GetTextureLevel()));
                } else if (readAttachment.IsRenderbuffer()) {
                    // TODO: renderbuffer support
                }
                MG_External::GLES::glReadBuffer(glAttachment);
            }
        }

        UnorderedMap<SharedPtr<MG_State::GLState::FramebufferObject>, SharedPtr<BackendFramebufferObject>>
            g_backendFramebufferObjects;
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        UnorderedMap<SharedPtr<MG_State::GLState::ProgramObject>, SharedPtr<BackendProgramObjectImpl>>
            g_backendProgramObjects;

        BackendProgramObjectImpl::BackendProgramObjectImpl() {
            m_backendProgramId = MG_External::GLES::glCreateProgram();
            if (m_backendProgramId == 0) {
                MGLOG_E("Failed to create program object in backend.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());

            } else {
                MGLOG_D("Created backend program object with ID: %u", m_backendProgramId);
            }
        }

        BackendProgramObjectImpl::~BackendProgramObjectImpl() {
            if (m_backendProgramId != 0) {
                MGLOG_D("Deleting backend program object with ID: %u", m_backendProgramId);
                MG_External::GLES::glDeleteProgram(m_backendProgramId);
            }
        }

        void BackendProgramObjectImpl::SyncToBackend(SharedPtr<MG_State::GLState::ProgramObject>& stateProgramObject) {
            if (!stateProgramObject) {
                MGLOG_E("State program object is null, skipping backend sync.");
                return;
            }

            if (!stateProgramObject->GetLinkStatus()) {
                MGLOG_E("Program object is not linked, skipping backend sync. State program ID: %u",
                        stateProgramObject->GetExternalIndex());
                return;
            }

            MGLOG_D("Syncing program to backend. State program ID: %u, Backend ID: %u",
                    stateProgramObject->GetExternalIndex(), m_backendProgramId);

            // Detach all existing shaders
            GLint attachedCount = 0;
            MG_External::GLES::glGetProgramiv(m_backendProgramId, GL_ATTACHED_SHADERS, &attachedCount);
            MGLOG_D("Currently attached shaders count: %d", attachedCount);

            if (attachedCount > 0) {
                Vector<GLuint> attachedShaders(attachedCount);
                GLsizei actualCount;
                MG_External::GLES::glGetAttachedShaders(m_backendProgramId, attachedCount, &actualCount,
                                                        attachedShaders.data());
                MGLOG_D("Detaching %d existing shaders from program %u", actualCount, m_backendProgramId);

                for (GLsizei i = 0; i < actualCount; ++i) {
                    MGLOG_D("Detaching shader ID: %u from program %u", attachedShaders[i], m_backendProgramId);
                    MG_External::GLES::glDetachShader(m_backendProgramId, attachedShaders[i]);
                }
            }

            // Attach current shaders
            auto& attachedShaders = stateProgramObject->GetAttachedShaders();
            MGLOG_D("Attaching %zu shaders to program %u", attachedShaders.size(), m_backendProgramId);
            for (auto& shader : attachedShaders) {
                const auto& src = shader->GetShaderSource();
                const auto& stage =
                    MG_Util::ConvertGLEnumToString(MG_Util::ConvertShaderStageToGLEnum(shader->GetShaderStage()));
                MGLOG_D("Original src @ %s: \n", stage.c_str());
                MGLOG_D("%s:", src.empty() ? "" : src.c_str());
            }
            auto& shaderSpirvs = stateProgramObject->GetGeneratedSpirv();

            for (int index = 0; index < attachedShaders.size(); ++index) {
                auto& shader = attachedShaders[index];
                GLenum glShaderType = MG_Util::ConvertShaderStageToGLEnum(shader->GetShaderStage());
                GLuint backendShaderId = MG_External::GLES::glCreateShader(glShaderType);

                if (backendShaderId == 0) {
                    MGLOG_E("Failed to create backend shader for attachment.");
                    continue;
                }
                String source;
                auto& spirvCode = shaderSpirvs[index];

                MG_Util::ShaderTranspiler::SpvcSession spvcSession(spirvCode);

                spvc_compiler_options options;
                spvcSession.CreateOptions(&options);

                spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 320);
                spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_TRUE);
                spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_VULKAN_SEMANTICS, SPVC_FALSE);

                spvcSession.SetOptions(options);

                const char* result = nullptr;
                spvcSession.Compile(&result);

                if (!result) {
                    MG_Util::ShaderTranspiler::ResultInfo r;
                    r.log += "Failed to compile the shader to GLSL: \n";
                    r.log += spvcSession.GetLastErrorString();
                    r.errc = -5;
                    MGLOG_E("%s", r.log.c_str());
                    continue;
                }

                source = result;

                source = RemoveLayoutBinding(source);
                source = ProcessOutColorLocations(source);
                source = ForceSupporterOutput(source);

                const char* sourceCStr = source.c_str();
                MGLOG_D("Setting shader source for backend shader ID: %u\nsrc:\n%s", backendShaderId, sourceCStr);
                MG_External::GLES::glShaderSource(backendShaderId, 1, &sourceCStr, nullptr);
                MG_External::GLES::glCompileShader(backendShaderId);

                GLint compileStatus;
                MG_External::GLES::glGetShaderiv(backendShaderId, GL_COMPILE_STATUS, &compileStatus);
                if (compileStatus == GL_FALSE) {
                    GLint logLength;
                    MG_External::GLES::glGetShaderiv(backendShaderId, GL_INFO_LOG_LENGTH, &logLength);
                    Vector<GLchar> log(logLength);
                    MG_External::GLES::glGetShaderInfoLog(backendShaderId, logLength, nullptr, log.data());
                    MGLOG_E("Shader compilation failed for backend ID %u: %s", backendShaderId, log.data());
                    continue;
                }

                MGLOG_D("Attaching shader ID: %u to program %u", backendShaderId, m_backendProgramId);
                MG_External::GLES::glAttachShader(m_backendProgramId, backendShaderId);

                MGLOG_D("Processed shader source length: %zu", source.length());
            }

            // Link program
            MGLOG_D("Linking program %u", m_backendProgramId);
            MG_External::GLES::glLinkProgram(m_backendProgramId);

            GLint linkStatus;
            MG_External::GLES::glGetProgramiv(m_backendProgramId, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint logLength;
                MG_External::GLES::glGetProgramiv(m_backendProgramId, GL_INFO_LOG_LENGTH, &logLength);
                Vector<GLchar> log(logLength);
                MG_External::GLES::glGetProgramInfoLog(m_backendProgramId, logLength, nullptr, log.data());
                MGLOG_E("Program %u linking failed for %u: %s", stateProgramObject->GetExternalIndex(),
                        m_backendProgramId, log.data());
            } else {
                MGLOG_D("Program linked successfully. ID: %u", m_backendProgramId);
            }

            // Create global UBO
            if (stateProgramObject->GetUBOSize() > 0) {
                MG_External::GLES::glGenBuffers(1, &m_backendGlobalUBOId);
                MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, m_backendGlobalUBOId);
                MG_External::GLES::glBufferData(GL_UNIFORM_BUFFER, stateProgramObject->GetUBOSize(), nullptr,
                                                GL_STREAM_DRAW);
                MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, 0);
            } else {
                m_backendGlobalUBOId = 0;
            }

            m_isInitialized = true;
            MGLOG_D("Program sync completed. backend ID %u", m_backendProgramId);
        }

        void BackendProgramObjectImpl::Use() {
            MGLOG_D("Using program %u", m_backendProgramId);
            MG_External::GLES::glUseProgram(m_backendProgramId);
        }
    } // namespace PrgramImpl

    namespace SamplerImpl {
        BackendSamplerObject::BackendSamplerObject() {
            MG_External::GLES::glGenSamplers(1, &m_backendSamplerId);
            if (m_backendSamplerId == 0) {
                MGLOG_E("Failed to generate sampler object.");
                MGLOG_E("ES glGetError(): %s", MG_Util::ConvertGLEnumToString(MG_External::GLES::glGetError()).c_str());
            } else {
                MGLOG_D("Generated sampler object with ID: %u.", m_backendSamplerId);
            }
        }

        void BackendSamplerObject::SyncToBackend(SharedPtr<MG_State::GLState::SamplerObject>& stateSamplerObject) {
            if (!stateSamplerObject) {
                MGLOG_E("State sampler object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing sampler with backend ID %u to backend for state ID %u", m_backendSamplerId,
                    stateSamplerObject->GetExternalIndex());

            const auto& samplerParams = stateSamplerObject->GetAllSamplerParameters();

#define SYNC_SAMPLER_PARAM_IF_CHANGED(internalName, glName, type)                                                      \
    if (m_cacheSamplerParameters.internalName != samplerParams.internalName) {                                         \
        MG_External::GLES::glSamplerParameteri(m_backendSamplerId, glName,                                             \
                                               MG_Util::ConvertSampler##type##ToGLEnum(samplerParams.internalName));   \
        m_cacheSamplerParameters.internalName = samplerParams.internalName;                                            \
    }

            if (m_cacheSamplerParameters.minFilter != samplerParams.minFilter ||
                m_cacheSamplerParameters.mipmapMode != samplerParams.mipmapMode) {
                MG_External::GLES::glSamplerParameteri(
                    m_backendSamplerId, GL_TEXTURE_MIN_FILTER,
                    MG_Util::ConvertSamplerFilterModeToGLEnum(samplerParams.minFilter, samplerParams.mipmapMode));
                m_cacheSamplerParameters.minFilter = samplerParams.minFilter;
                m_cacheSamplerParameters.mipmapMode = samplerParams.mipmapMode;
            }
            if (m_cacheSamplerParameters.magFilter != samplerParams.magFilter) {
                MG_External::GLES::glSamplerParameteri(
                    m_backendSamplerId, GL_TEXTURE_MAG_FILTER,
                    MG_Util::ConvertSamplerFilterModeToGLEnum(samplerParams.magFilter, SamplerMipmapMode::None));
                m_cacheSamplerParameters.magFilter = samplerParams.magFilter;
            }

            SYNC_SAMPLER_PARAM_IF_CHANGED(wrapS, GL_TEXTURE_WRAP_S, WrapMode)
            SYNC_SAMPLER_PARAM_IF_CHANGED(wrapT, GL_TEXTURE_WRAP_T, WrapMode)
            SYNC_SAMPLER_PARAM_IF_CHANGED(wrapR, GL_TEXTURE_WRAP_R, WrapMode)
            SYNC_SAMPLER_PARAM_IF_CHANGED(compareFunc, GL_TEXTURE_COMPARE_FUNC, CompareFunc)
            SYNC_SAMPLER_PARAM_IF_CHANGED(compareMode, GL_TEXTURE_COMPARE_MODE, CompareMode)
            if (m_cacheSamplerParameters.minLod != samplerParams.minLod) {
                MG_External::GLES::glSamplerParameterf(m_backendSamplerId, GL_TEXTURE_MIN_LOD, samplerParams.minLod);
                m_cacheSamplerParameters.minLod = samplerParams.minLod;
            }
            if (m_cacheSamplerParameters.maxLod != samplerParams.maxLod) {
                MG_External::GLES::glSamplerParameterf(m_backendSamplerId, GL_TEXTURE_MAX_LOD, samplerParams.maxLod);
                m_cacheSamplerParameters.maxLod = samplerParams.maxLod;
            }
#undef SYNC_SAMPLER_PARAM_IF_CHANGED
            m_isInitialized = true;
        }

        void BackendSamplerObject::Bind(Uint unit) {
            MG_External::GLES::glBindSampler(static_cast<GLenum>(unit), m_backendSamplerId);
        }

        Uint BackendSamplerObject::GetBackendSamplerId() {
            return m_backendSamplerId;
        }

        UnorderedMap<SharedPtr<MG_State::GLState::SamplerObject>, SharedPtr<BackendSamplerObject>>
            g_backendSamplerObjects;
    } // namespace SamplerImpl

    namespace Utils {} // namespace Utils
} // namespace MobileGL::MG_Backend::DirectGLES
