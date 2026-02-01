// MobileGL - MobileGL/MG_Backend/DirectGLES/Managers.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "DirectGLES.h"
#include "MG_State/GLState/SamplerState/SamplerObject.h"
#include "MG_State/GLState/TextureState/TextureEnum.h"
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace BufferImpl {
        class BackendBufferObject {
        public:
            BackendBufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            Uint GetBackendBufferId() { return m_backendBufferId; }
            void Bind();
            void Bind(GLenum target);

        private:
            void SyncToBackend_glBufferData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glBufferSubData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glMapBufferRange(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject,
                                                Bool invalidate = true, Bool unsynchronized = true);

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
            void SyncToBackend(SharedPtr<MG_State::GLState::VertexArrayObject>& stateVAOObject, Bool needDivisor);
            Uint GetBackendVertexArrayId() { return m_backendVAOId; }
            void Bind();

        private:
            Uint m_backendVAOId = 0;
            Bool m_isInitialized = false;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::VertexArrayObject>, SharedPtr<BackendVertexArrayObject>>
            g_backendVertexArrayObjects;
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        inline Bool IsSupportedTextureTarget(TextureTarget target) {
            if (target == TextureTarget::Texture1D || target == TextureTarget::TextureRectangle ||
                target == TextureTarget::Texture2DMultisampleArray || target == TextureTarget::Texture1DArray ||
                target == TextureTarget::Texture2DMultisample || target == TextureTarget::Texture2DArray)
                return false;
            return true;
        }

        struct StateTextureBasicInfo { // Used for tracking texture state changes
            TextureInternalFormat internalFormat = TextureInternalFormat::Unknown;
            SizeT width = 0;
            SizeT height = 0;
            SizeT depth = 0;
            SizeT mipmapLevels = 0;
            Uint bufferExternalIndex = 0;

            bool operator==(const StateTextureBasicInfo& other) const {
                return internalFormat == other.internalFormat && width == other.width && height == other.height &&
                       depth == other.depth && mipmapLevels == other.mipmapLevels &&
                       bufferExternalIndex == other.bufferExternalIndex;
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
            SamplerParameters m_cacheSamplerParameters;
            UintVec2 m_cacheLodRange = {0, 1000};
            FloatVec4 m_cacheBorderColor = {0.0f, 0.0f, 0.0f, 0.0f};
            Vec4<TextureSwizzleParam> m_cacheSwizzleParams = {TextureSwizzleParam::Red, TextureSwizzleParam::Green,
                                                              TextureSwizzleParam::Blue, TextureSwizzleParam::Alpha};
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::ITextureObject>, SharedPtr<BackendTextureObject>>
            g_backendTextureObjects;
    } // namespace TextureImpl

    namespace FramebufferImpl {
        class BackendFramebufferObject {
        public:
            BackendFramebufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::FramebufferObject>& stateFBOObject,
                               FramebufferTarget asTarget);
            Uint GetBackendFramebufferId() { return m_backendFBOId; }
            void Bind(FramebufferTarget target);
            FramebufferAttachmentType GetCompactedAttachmentTypeAtDrawBufferIndex(Int index);

        private:
            Uint m_backendFBOId = 0;

            /* this will save buffers in its original form,
               reversion, absence or not consecutive are all allowed, as long as GL spec allows it
               i.e. it could be like [COLOR_ATTACHMENT0, COLOR_ATTACHMENT5, NONE, COLOR_ATTACHMENT4]
               Probably useful to re-link shader output according to this.
               aka. realizing `glBindFragDataLocation`
             */
            FramebufferAttachmentType m_frontendDrawBuffers[MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS] = {
                FramebufferAttachmentType::None};
            /* this will save buffers in its compacted GL form,
               not consecutive is not allowed
               i.e. it could be like [COLOR_ATTACHMENT0, COLOR_ATTACHMENT5, COLOR_ATTACHMENT4]
               (no GL_NONE among those)
             */
            FramebufferAttachmentType
                m_compactedFrontendDrawBuffers[MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS] = {
                    FramebufferAttachmentType::None};
            /* this will save buffers in stricter ES rules
               reversion, absence or not consecutive are not allowed, according to ES spec
               i.e. it could be like [COLOR_ATTACHMENT0, COLOR_ATTACHMENT1, NONE, NONE, ...]
               this array could be provided as data directly to ES `glDrawBuffers` function
             */
            GLenum m_backendDrawBuffers[MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS] = {GL_NONE};
            FramebufferAttachmentType m_frontendReadBuffer = FramebufferAttachmentType::Color0;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::FramebufferObject>, SharedPtr<BackendFramebufferObject>>
            g_backendFramebufferObjects;
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        class BackendProgramObjectImpl {
        public:
            BackendProgramObjectImpl();
            ~BackendProgramObjectImpl();
            void SyncToBackend(SharedPtr<MG_State::GLState::ProgramObject>& stateProgramObject);
            void Use();
            Uint GetBackendProgramId() const { return m_backendProgramId; }
            Uint GetBackendGlobalUBOId() const { return m_backendGlobalUBOId; }

        private:
            Uint m_backendProgramId = 0;
            Uint m_backendGlobalUBOId = 0;
            Bool m_isInitialized = false;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::ProgramObject>, SharedPtr<BackendProgramObjectImpl>>
            g_backendProgramObjects;
    } // namespace PrgramImpl

    namespace SamplerImpl {
        class BackendSamplerObject {
        public:
            BackendSamplerObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::SamplerObject>& stateSamplerObject);
            void Bind(Uint unit);
            Uint GetBackendSamplerId();

        private:
            Uint m_backendSamplerId = 0;
            Bool m_isInitialized = false;
            SamplerParameters m_cacheSamplerParameters;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::SamplerObject>, SharedPtr<BackendSamplerObject>>
            g_backendSamplerObjects;
    } // namespace SamplerImpl

    namespace RenderbufferImpl {
        class BackendRenderbufferObject {
        public:
            BackendRenderbufferObject();
            void SyncToBackend(const SharedPtr<MG_State::GLState::RenderbufferObject>& stateRBOObject);
            Uint GetBackendRenderbufferId() { return m_backendRBOId; }
            void Bind();

        private:
            Uint m_backendRBOId = 0;
            Bool m_isInitialized = false;
            TextureInternalFormat m_cacheInternalFormat = TextureInternalFormat::Unknown;
            Int m_cacheWidth = 0;
            Int m_cacheHeight = 0;
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::RenderbufferObject>, SharedPtr<BackendRenderbufferObject>>
            g_backendRenderbufferObjects;
    } // namespace RenderbufferImpl
} // namespace MobileGL::MG_Backend::DirectGLES
