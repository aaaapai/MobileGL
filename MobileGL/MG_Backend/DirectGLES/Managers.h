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
#include "Utils.h"
#include "MG_State/GLState/SamplerState/SamplerObject.h"
#include "MG_State/GLState/TextureState/TextureEnum.h"
#include <MG_State/GLState/TextureState/TextureObject.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace BufferImpl {
        const GLenum TempBufferTarget = GL_ARRAY_BUFFER;
        class BackendBufferObject {
        public:
            BackendBufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            Uint GetBackendBufferId() { return m_backendBufferId; }
            void Bind(GLenum target = TempBufferTarget);

        private:
            void SyncToBackend_glBufferData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glBufferSubData(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject);
            void SyncToBackend_glMapBufferRange(SharedPtr<MG_State::GLState::BufferObject>& stateBufferObject,
                                                Bool invalidate = true, Bool unsynchronized = true);

            Uint m_backendBufferId = 0;
            SizeT m_prevBufferSize = 0;
            Bool m_isInitialized = false;
        };

        extern BackendBufferObject* g_boundVertexBufferObject;
        extern UnorderedMap<SharedPtr<MG_State::GLState::BufferObject>, SharedPtr<BackendBufferObject>>
            g_backendBufferObjects;
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        class BackendVertexArrayObject {
        public:
            BackendVertexArrayObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::VertexArrayObject>& stateVAOObject);
            Uint GetBackendVertexArrayId() { return m_backendVAOId; }
            void Bind();

        private:
            void SyncAttributeBuffer(Uint index, const MG_State::GLState::VertexAttribute& attrib);

            Uint m_backendVAOId = 0;
            Bool m_isInitialized = false;
            Uint16 m_syncedIndexBufferVersion = 0;
            Array<MG_State::GLState::VertexAttributeVersion, MG_State::GLState::VertexArrayObject::MAX_VERTEX_ATTRIBS>
                m_syncedAttributeVersions;
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

        inline const Uint TempTextureUnit = 0;
        class BackendTextureObject {
        public:
            BackendTextureObject();
            void SyncMipmapsToBackend(SharedPtr<MG_State::GLState::ITextureObject>& stateTextureObject);
            void SyncBuiltinSamplerToBackend(SharedPtr<MG_State::GLState::ITextureObject>& stateTextureObject);
            void SyncTextureParamsToBackend(SharedPtr<MG_State::GLState::ITextureObject>& stateTextureObject);
            void Bind(GLenum target, Uint unit = TempTextureUnit);
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
            Uint16 m_syncedSamplerVersion = 0;
            Uint16 m_syncedTextureParamsVersion = 0;
        };

        void ActivateTextureUnit(Uint unit);
        void UnbindTexture(Uint unit, GLenum target);
        extern UnorderedMap<SharedPtr<MG_State::GLState::ITextureObject>, SharedPtr<BackendTextureObject>>
            g_backendTextureObjects;
        extern Array<Array<BackendTextureObject*, (SizeT)TextureTarget::TextureTargetCount>,
                     MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS>
            g_boundTexturesCache;
        extern Uint g_activeTextureUnit;
    } // namespace TextureImpl

    namespace FramebufferImpl {
        class BackendFramebufferObject {
        public:
            BackendFramebufferObject();
            void SyncToBackend(SharedPtr<MG_State::GLState::FramebufferObject>& stateFBOObject,
                               FramebufferTarget asTarget);
            Uint GetBackendFramebufferId() { return m_backendFBOId; }
            void Bind(FramebufferTarget target);
            bool SyncAttachmentObject(GLenum glFBOTarget,
                                      const MG_State::GLState::FramebufferAttachmentObject& attachmentObject,
                                      GLenum glBackendAttachment);
//            FramebufferAttachmentType GetCompactedAttachmentTypeAtDrawBufferIndex(Int index);
            GLenum GetBackendAttachmentType(FramebufferAttachmentType frontendAtt) const;
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
            /* this will save buffers in stricter ES rules
               reversion, absence or not consecutive are not allowed, according to ES spec
               i.e. it could be like [COLOR_ATTACHMENT0, COLOR_ATTACHMENT1, NONE, COLOR_ATTACHMENT3, ...]
               this array could be provided as data directly to ES `glDrawBuffers` function
             */
            GLenum m_backendDrawBuffers[MG_State::GLState::FramebufferObject::MAX_DRAW_BUFFERS] = {GL_NONE};
            FramebufferAttachmentType m_frontendReadBuffer = FramebufferAttachmentType::Color0;
            GLenum m_backendReadBuffer = GL_COLOR_ATTACHMENT0;

            using FramebufferObject = MG_State::GLState::FramebufferObject;
            FramebufferObject::FramebufferAttachmentVersionArray m_syncedFrontendAttachmentVersions = {0};
        };

        extern UnorderedMap<SharedPtr<MG_State::GLState::FramebufferObject>, SharedPtr<BackendFramebufferObject>>
            g_backendFramebufferObjects;
        extern Array<Uint16, SizeT(FramebufferTarget::FramebufferTargetCount)> g_fboBindVersions;
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
            Uint16 m_syncedSamplerVersion = 0;
        };

        void UnbindSampler(Uint unit);

        extern Array<BackendSamplerObject*, MG_State::GLState::TextureState::MAX_TEXTURE_IMAGE_UNITS>
            g_boundSamplersCache;
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
