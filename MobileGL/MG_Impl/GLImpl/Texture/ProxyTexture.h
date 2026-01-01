// MobileGL - MobileGL/MG_Impl/GLImpl/Texture/ProxyTexture.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
