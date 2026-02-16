// MobileGL - MobileGL/MG_Backend/DirectVulkan/DirectVulkan.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "DirectVulkan.h"
#include "TmpImpl.h"
constexpr bool USE_TMP_IMPL = true;

namespace MobileGL::MG_Backend::DirectVulkan {
    UniquePtr<VulkanRenderer> pVulkanRenderer = nullptr;

    void Clear(GLbitfield mask) {
        if (USE_TMP_IMPL) {
            MobileGL::MG_Backend::DirectVulkan::TmpImpl::Clear(mask);
            return;
        }
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        if (USE_TMP_IMPL) {
            MobileGL::MG_Backend::DirectVulkan::TmpImpl::DrawElements(mode, count, type, indices);
            return;
        }
        pVulkanRenderer->RenderFrame();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan