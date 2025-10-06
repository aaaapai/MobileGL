#pragma once
#include <Includes.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Impl::GLImpl {
    namespace TextureImpl {
        Bool IsProxyTextureTarget(TextureUploadTarget target);

        class ProxyTextureManager {
        public:
            SharedPtr<MG_State::GLState::ITextureObject> CreateOrReplaceProxyTextureObject(TextureUploadTarget target);
            SharedPtr<MG_State::GLState::ITextureObject> GetProxyTextureObject(TextureUploadTarget target);

        private:
            UnorderedMap<TextureUploadTarget, SharedPtr<MG_State::GLState::ITextureObject>> m_proxyTexturesMap;
        };

        extern ProxyTextureManager* pProxyTextureManager;
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl
