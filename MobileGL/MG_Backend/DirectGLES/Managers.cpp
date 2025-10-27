#include "Managers.h"
#include "MG_Backend/Backends.h"
#include "MG_State/GLState/ProgramState/ShaderObject.h"
#include "MG_Util/Types.h"
#include "Utils.h"
#include "DirectGLES.h"
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/DataTypeConverter.h>
#include <MG_Util/Converters/MGToGL/BufferEnumConverter.h>
#include <MG_Util/Converters/MGToGL/TextureEnumConverter.h>
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

            MGLOG_D("Syncing buffer object with ID: %u to backend for state: %s", m_backendBufferId,
                    stateBufferObject.get());

            Bool needsRegeneration =
                !m_isInitialized || bufferSize > m_prevBufferSize || bufferSize > m_prevBufferSize * 2;

            if (needsRegeneration) {
                MGLOG_D("Buffer size changed significantly or not initialized, regenerating buffer with ID: %u",
                        m_backendBufferId);
                SyncToBackend_glBufferData(stateBufferObject);
                m_isInitialized = true;
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
            const auto& range = stateBufferObject->GetDirtyRange();
            // dirty range: [range.start, range.end)

            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
            MG_External::GLES::glBufferSubData(TempBufferTarget, range.start, range.end - range.start,
                                               reinterpret_cast<const char*>(data) + range.start);
        }

        void BackendBufferObject::SyncToBackend_glMapBufferRange(
            SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject, Bool invalidate) {
            BackendBufferBindingProtector backendBufferBindingProtector(TempBufferTarget);

            MGLOG_D("Syncing buffer map (glMapBuffer) for object with ID : %u", m_backendBufferId);
            MGLOG_D("Mapping buffer with ID: %u", m_backendBufferId);
            MG_External::GLES::glBindBuffer(TempBufferTarget, m_backendBufferId);
            const auto& range = stateBufferObject->GetDirtyRange();
            void* mappedData = MG_External::GLES::glMapBufferRange(
                TempBufferTarget, range.start, range.end - range.start,
                (invalidate ? GL_MAP_INVALIDATE_BUFFER_BIT : 0) | GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
            const void* data = stateBufferObject->GetDataReadOnly()->data();
            if (mappedData) {
                Memcpy(reinterpret_cast<const void*>(reinterpret_cast<const char*>(data) + range.start), mappedData,
                       range.end - range.start);
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

        void BackendVertexArrayObject::SyncToBackend(SharedPtr<MG_State::GLState::VertexArrayObject>& stateVAOObject) {
            if (!stateVAOObject) {
                MGLOG_E("State VAO object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing VAO object with ID: %u to backend for state: 0x%p", m_backendVAOId, stateVAOObject.get());

            BufferImpl::BackendBufferBindingProtector backendBufferBindingProtector(BufferImpl::TempBufferTarget);
            BackendVertexArrayBindingProtector backendVAOBindingProtector;

            Bind();

            for (const auto& attribIndex : stateVAOObject->GetDirtyAttributeIndices()) {
                const auto& attrib = stateVAOObject->GetAttribute(attribIndex);

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
                MG_External::GLES::glEnableVertexAttribArray(attribIndex);
                MG_External::GLES::glVertexAttribPointer(
                    attribIndex, attrib.Size, MG_Util::ConvertDataTypeToGLEnum(attrib.Type),
                    attrib.Normalized ? GL_TRUE : GL_FALSE, attrib.Stride,
                    reinterpret_cast<const void*>(static_cast<SizeT>(attrib.Offset)));

                // TODO: divisor
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
            if (!stateTextureObject) {
                MGLOG_E("State texture object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing texture object with ID: %u to backend for state: 0x%p", m_backendTextureId,
                    stateTextureObject.get());

            GLenum target = MG_Util::ConvertTextureTargetToGLEnum(stateTextureObject->GetTarget());

            // The texture needs to be regenerated completely with glTexImage* calls if:
            // 1. Not initialized
            // 2. InternalFormat changed
            // 3. Size changed
            // 4. Mipmap levels changed

            if (!stateTextureObject->IsComplete()) {
                MGLOG_E("Texture object with ID: %u is not complete, skipping sync.", m_backendTextureId);
                return;
            }

            BackendTextureBindingProtector backendTextureBindingProtector(target);
            Bind(target);

            StateTextureBasicInfo currentTextureInfo = {
                stateTextureObject->GetFormat(), static_cast<SizeT>(stateTextureObject->GetBaseSize().x()),
                static_cast<SizeT>(stateTextureObject->GetBaseSize().y()),
                static_cast<SizeT>(stateTextureObject->GetBaseSize().z()), stateTextureObject->GetMipmaps().size()};

            Bool needsRegeneration = !m_isInitialized || (currentTextureInfo != m_prevTextureInfo);

            if (needsRegeneration) {
                MGLOG_D("Texture state changed significantly or not initialized, regenerating texture with ID: %u",
                        m_backendTextureId);

                // Regenerate all mipmap levels
                const auto& mipmaps = stateTextureObject->GetMipmaps();
                GLenum glInternalFormat, glType, glFormat;
                TextureImpl::GenerateTextureFormatInfo(stateTextureObject->GetFormat(), &glInternalFormat, &glType,
                                                       &glFormat);
                for (SizeT level = 0; level < mipmaps.size(); ++level) {
                    const auto& mipmap = mipmaps[level];

                    MG_External::GLES::glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), glInternalFormat,
                                                    static_cast<GLsizei>(mipmap.size.x()),
                                                    static_cast<GLsizei>(mipmap.size.y()), 0, glFormat, glType,
                                                    mipmap.data.data());

                    MGLOG_D("Regenerated mipmap level %d for texture with ID: %u", level, m_backendTextureId);
                    stateTextureObject->UnmarkMipmapDirty(level);
                }

                m_isInitialized = true;
            }

            { // Update sampler parameters; TODO: always use sampler objects in backend

                const auto& samplerObject = stateTextureObject->GetSamplerObject();
                if (samplerObject) {
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_MIN_FILTER,
                        MG_Util::ConvertSamplerFilterModeToGLEnum(samplerObject->GetMinFilter()));
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_MAG_FILTER,
                        MG_Util::ConvertSamplerFilterModeToGLEnum(samplerObject->GetMagFilter()));
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_WRAP_S, MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObject->GetWrapS()));
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_WRAP_T, MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObject->GetWrapT()));
                    MG_External::GLES::glTexParameteri(
                        target, GL_TEXTURE_WRAP_R, MG_Util::ConvertSamplerWrapModeToGLEnum(samplerObject->GetWrapR()));
                }
            }

            { // Update all dirty mipmap levels
                const auto& mipmaps = stateTextureObject->GetMipmaps();
                GLenum glInternalFormat, glType, glFormat;
                TextureImpl::GenerateTextureFormatInfo(stateTextureObject->GetFormat(), &glInternalFormat, &glType,
                                                       &glFormat);
                for (const auto& mipmap : stateTextureObject->GetMipmaps()) {
                    if (!mipmap.dirty) {
                        continue;
                    }

                    if (mipmap.data.empty()) {
                        MGLOG_W("Mipmap level %d has no data, skipping update.", mipmap.level);
                        continue;
                    }

                    MG_External::GLES::glTexSubImage2D(
                        GL_TEXTURE_2D, static_cast<GLint>(mipmap.level), 0, 0, static_cast<GLsizei>(mipmap.size.x()),
                        static_cast<GLsizei>(mipmap.size.y()), glFormat, glType, mipmap.data.data());
                    stateTextureObject->UnmarkMipmapDirty(mipmap.level);
                }
            }

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

        void BackendFramebufferObject::Bind() {
            MG_External::GLES::glBindFramebuffer(GL_FRAMEBUFFER, m_backendFBOId);
        }

        void BackendFramebufferObject::SyncToBackend(SharedPtr<MG_State::GLState::FramebufferObject>& stateFBOObject) {
            if (!stateFBOObject) {
                MGLOG_E("State FBO object is null, cannot sync to backend.");
                return;
            }

            MGLOG_D("Syncing FBO object with ID: %u to backend for state: 0x%p", m_backendFBOId, stateFBOObject.get());

            BackendFramebufferBindingProtector backendFBOBindingProtector(GL_FRAMEBUFFER);
            Bind();

            // TODO: add dirty check
            // Sync all attachments
            const auto& attachments = stateFBOObject->GetAllAttachments();
            for (SizeT index = 0; index < attachments.size(); ++index) {
                FramebufferAttachmentType attachmentType = static_cast<FramebufferAttachmentType>(index);
                const auto& attachment = attachments[index];
                if (!attachment.IsComplete()) {
                    continue;
                }

                if (attachment.IsTexture()) {
                    const auto& textureObject = attachment.GetTexture();
                    const auto& backendTextureIt = TextureImpl::g_backendTextureObjects.find(textureObject);
                    if (backendTextureIt == TextureImpl::g_backendTextureObjects.end()) {
                        MGLOG_E("No backend texture found for FBO attachment, cannot bind texture.");
                        continue;
                    }
                    const auto& backendTextureObject = backendTextureIt->second;
                    backendTextureObject->Bind(MG_Util::ConvertTextureTargetToGLEnum(textureObject->GetTarget()));
                    MG_External::GLES::glFramebufferTexture2D(
                        GL_FRAMEBUFFER, MG_Util::ConvertFramebufferAttachmentTypeToGLEnum(attachmentType),
                        MG_Util::ConvertTextureTargetToGLEnum(textureObject->GetTarget()),
                        backendTextureObject->GetBackendTextureId(), static_cast<GLint>(attachment.GetTextureLevel()));
                } else if (attachment.IsRenderbuffer()) {
                    // TODO
                }
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
                MGLOG_E("Program object is not linked, skipping backend sync. Program: 0x%p", stateProgramObject.get());
                return;
            }

            MGLOG_D("Syncing program to backend. State program: 0x%p, Backend ID: 0x%p", stateProgramObject.get(),
                    m_backendProgramId);

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
            auto& shaderSpirvs = stateProgramObject->GetGeneratedSpirv();

            for (int index = 0; index < attachedShaders.size(); ++index) {
                auto& shader = attachedShaders[index];
                GLenum glShaderType = MG_State::GLState::ConvertGLShaderTypeByMGLShaderStage(shader->GetShaderStage());
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
                MGLOG_D("Setting shader source for backend shader ID: %u", backendShaderId);
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
                MGLOG_E("Program linking failed for %u: %s", m_backendProgramId, log.data());
            } else {
                MGLOG_D("Program linked successfully. ID: %u", m_backendProgramId);
            }

            // Create global UBO
            MG_External::GLES::glGenBuffers(1, &m_backendGlobalUBOId);
            MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, m_backendGlobalUBOId);
            MG_External::GLES::glBufferData(GL_UNIFORM_BUFFER, stateProgramObject->GetUBOSize(), nullptr,
                                            GL_STREAM_DRAW);
            MG_External::GLES::glBindBuffer(GL_UNIFORM_BUFFER, 0);

            // Bind samplers


            m_isInitialized = true;
            MGLOG_D("Program sync completed. Backend ID: %u", m_backendProgramId);
        }

        void BackendProgramObjectImpl::Use() {
            MGLOG_D("Using program %u", m_backendProgramId);
            MG_External::GLES::glUseProgram(m_backendProgramId);
        }
    } // namespace PrgramImpl

    namespace Utils {} // namespace Utils
} // namespace MobileGL::MG_Backend::DirectGLES
