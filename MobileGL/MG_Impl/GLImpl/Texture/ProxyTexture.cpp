#include "ProxyTexture.h"
#include "MG_State/GLState/TextureState/TextureObject2D.h"
#include "MG_Util/Types.h"

namespace MobileGL::MG_Impl::GLImpl {
    namespace TextureImpl {
        ProxyTextureManager* pProxyTextureManager;

        Bool IsProxyTextureTarget(TextureUploadTarget target) {
            switch (target) {
            case TextureUploadTarget::ProxyCubeMap:
            case TextureUploadTarget::ProxyTexture1DArray:
            case TextureUploadTarget::ProxyTexture2D:
            case TextureUploadTarget::ProxyTexture2DMultisample:
            case TextureUploadTarget::ProxyTextureRectangle:
                return true;
            default:
                return false;
            }
        }

        SharedPtr<MG_State::GLState::ITextureObject> ProxyTextureManager::CreateOrReplaceProxyTextureObject(
            TextureUploadTarget target) {
            auto it = m_proxyTexturesMap.find(target);
            if (it != m_proxyTexturesMap.end()) {
                m_proxyTexturesMap.erase(it);
            }
            m_proxyTexturesMap[target] = MakeShared<MG_State::GLState::TextureObject2D>(0);
            return m_proxyTexturesMap[target];
        }

        SharedPtr<MG_State::GLState::ITextureObject> ProxyTextureManager::GetProxyTextureObject(
            TextureUploadTarget target) {
            auto it = m_proxyTexturesMap.find(target);
            if (it != m_proxyTexturesMap.end()) {
                return it->second;
            }
            return nullptr;
        }
    } // namespace TextureImpl
} // namespace MobileGL::MG_Impl::GLImpl
