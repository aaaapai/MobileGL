// MobileGL - MobileGL/MG_Backend/DirectVulkan/DirectVulkan.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "DirectVulkan.h"
#include "MG_State/GLState/Core.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    UniquePtr<VulkanRenderer> pVulkanRenderer = nullptr;

    void Clear(GLbitfield mask) {
        if (!pVulkanRenderer || !MG_State::pGLContext) {
            return;
        }

        const auto& clearColor = MG_State::pGLContext->GetClearColor();
        const auto clearDepth = MG_State::pGLContext->GetClearDepth();
        const auto clearStencil = static_cast<Uint32>(MG_State::pGLContext->GetClearStencil());
        pVulkanRenderer->RequestClear(mask, clearColor, clearDepth, clearStencil);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        if (pVulkanRenderer) {
            pVulkanRenderer->EnsureFrameRecordingStarted();
        }

        (void)mode;
        (void)count;
        (void)type;
        (void)indices;
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
