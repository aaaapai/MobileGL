#pragma once
#include <Includes.h>
#include "DirectGLES.h"
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace BufferImpl {
        class BackendBufferObject {
        public:
            BackendBufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void Bind();
            void Bind(GLenum target);

        private:
            void SyncToBackend_glBufferData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glBufferSubData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glMapBufferRange(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject,
                                                Bool invalidate = false);

            Uint m_backendBufferId = 0;
            SizeT m_prevBufferSize = 0;
            Bool m_isInitialized = false;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::BufferObject>, SharedPtr<BackendBufferObject>>
            g_backendBufferObjects;
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        class BackendVertexArrayObject {
        public:
            BackendVertexArrayObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::VertexArrayObject>& stateVAOObject);
            void Bind();

        private:
            Uint m_backendVAOId = 0;
            Bool m_isInitialized = false;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::VertexArrayObject>, SharedPtr<BackendVertexArrayObject>>
            g_backendVertexArrayObjects;
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        struct StateTextureBasicInfo { // Used for tracking texture state changes
            TextureInternalFormat internalFormat = TextureInternalFormat::Unknown;
            SizeT width = 0;
            SizeT height = 0;
            SizeT depth = 0;
            SizeT mipmapLevels = 0;

            bool operator==(const StateTextureBasicInfo& other) const {
                return internalFormat == other.internalFormat && width == other.width && height == other.height &&
                       depth == other.depth && mipmapLevels == other.mipmapLevels;
            }

            bool operator!=(const StateTextureBasicInfo& other) const { return !(*this == other); }
        };

        class BackendTextureObject {
        public:
            BackendTextureObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::ITextureObject>& stateTextureObject);
            void Bind(GLenum target);
            Uint GetBackendTextureId();

        private:
            Uint m_backendTextureId = 0;
            Bool m_isInitialized = false;
            StateTextureBasicInfo m_prevTextureInfo;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::ITextureObject>, SharedPtr<BackendTextureObject>>
            g_backendTextureObjects;
    } // namespace TextureImpl

    namespace FramebufferImpl {
        class BackendFramebufferObject {
        public:
            BackendFramebufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::FramebufferObject>& stateFBOObject);
            void Bind();

        private:
            Uint m_backendFBOId = 0;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::FramebufferObject>, SharedPtr<BackendFramebufferObject>>
            g_backendFramebufferObjects;
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        class BackendShaderObjectImpl {
        public:
            BackendShaderObjectImpl();
            ~BackendShaderObjectImpl();
            void SyncToBackend(SharedPtr<MG_State::GLState::ShaderObject>& stateShaderObject);
            Uint GetBackendShaderId() const { return m_backendShaderId; }

        private:
            Uint m_backendShaderId = 0;
            Bool m_isInitialized = false;
        };
        class BackendProgramObjectImpl {
        public:
            BackendProgramObjectImpl();
            ~BackendProgramObjectImpl();
            void SyncToBackend(SharedPtr<MG_State::GLState::ProgramObject>& stateProgramObject);
            void Use();
            Uint GetBackendProgramId() const { return m_backendProgramId; }

        private:
            Uint m_backendProgramId = 0;
            Bool m_isInitialized = false;
        };
        extern UnorderedMap<SharedPtr<MG_State::GLState::ShaderObject>, SharedPtr<BackendShaderObjectImpl>>
            g_backendShaderObjects;
        extern UnorderedMap<SharedPtr<MG_State::GLState::ProgramObject>, SharedPtr<BackendProgramObjectImpl>>
            g_backendProgramObjects;
    } // namespace PrgramImpl
} // namespace MobileGL::MG_Backend::DirectGLES