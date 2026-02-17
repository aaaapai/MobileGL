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
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::Clear called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::Clear called with null GL context");

        const auto& clearColor = MG_State::pGLContext->GetClearColor();
        const auto clearDepth = MG_State::pGLContext->GetClearDepth();
        const auto clearStencil = static_cast<Uint32>(MG_State::pGLContext->GetClearStencil());
        pVulkanRenderer->RequestClear(mask, clearColor, clearDepth, clearStencil);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawElements called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawElements called with null GL context");

        if (count < 0) {
            MGLOG_W("DrawElements skipped: count (%d) must be non-negative", count);
            return;
        }

        if (count == 0) {
            return;
        }

        if (mode != GL_TRIANGLES) {
            MGLOG_W("DrawElements skipped: primitive mode %u is not supported yet", mode);
            return;
        }

        SizeT indexSize = 0;
        switch (type) {
        case GL_UNSIGNED_SHORT:
            indexSize = sizeof(Uint16);
            break;
        case GL_UNSIGNED_INT:
            indexSize = sizeof(Uint32);
            break;
        default:
            MGLOG_W("DrawElements skipped: index type %u is not supported yet", type);
            return;
        }

        const auto vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MGLOG_W("DrawElements skipped: no bound VAO");
            return;
        }

        const auto indexBuffer = vao->GetIndexBufferBindingSlot().GetBoundObject();
        if (!indexBuffer) {
            MGLOG_W("DrawElements skipped: no bound ELEMENT_ARRAY_BUFFER");
            return;
        }

        const auto indexData = indexBuffer->GetDataReadOnly();
        if (!indexData || indexData->empty()) {
            MGLOG_W("DrawElements skipped: ELEMENT_ARRAY_BUFFER has no data");
            return;
        }

        const SizeT byteOffset = reinterpret_cast<SizeT>(indices);
        const SizeT requiredBytes = static_cast<SizeT>(count) * indexSize;
        if (byteOffset + requiredBytes > indexBuffer->GetSize()) {
            MGLOG_W("DrawElements skipped: index range out of bounds (offset=%zu, size=%zu, buffer=%zu)",
                    byteOffset, requiredBytes, indexBuffer->GetSize());
            return;
        }

        const Uint8* src = indexData->data() + byteOffset;
        pVulkanRenderer->DrawElements(type, count, src, requiredBytes);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawArrays called with null VulkanRenderer");

        if (first < 0) {
            MGLOG_W("DrawArrays skipped: first (%d) must be non-negative", first);
            return;
        }

        if (count < 0) {
            MGLOG_W("DrawArrays skipped: count (%d) must be non-negative", count);
            return;
        }

        if (count == 0) {
            return;
        }

        if (mode != GL_TRIANGLES) {
            MGLOG_W("DrawArrays skipped: primitive mode %u is not supported yet", mode);
            return;
        }

        pVulkanRenderer->DrawArrays(mode, first, count);
    }
} // namespace MobileGL::MG_Backend::DirectVulkan

