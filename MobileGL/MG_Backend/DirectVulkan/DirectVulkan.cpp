// MobileGL - MobileGL/MG_Backend/DirectVulkan/DirectVulkan.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "DirectVulkan.h"
#include "MG_State/GLState/Core.h"
#include "MG_Impl/GLImpl/Framebuffer/GL_Framebuffer.h"

namespace MobileGL::MG_Backend::DirectVulkan {
    UniquePtr<VulkanRenderer> pVulkanRenderer = nullptr;

    void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {}
    void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {}
    void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {}
    void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {}
    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawElementsBaseVertex called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawElementsBaseVertex called with null GL context");

        if (count < 0) {
            MGLOG_W("DrawElementsBaseVertex skipped: count (%d) must be non-negative", count);
            return;
        }
        if (count == 0) {
            return;
        }
        if (mode != GL_TRIANGLES) {
            MGLOG_W("DrawElementsBaseVertex skipped: primitive mode %u is not supported yet", mode);
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
            MGLOG_W("DrawElementsBaseVertex skipped: index type %u is not supported yet", type);
            return;
        }

        const auto vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MGLOG_W("DrawElementsBaseVertex skipped: no bound VAO");
            return;
        }

        const auto indexBuffer = vao->GetIndexBufferBindingSlot().GetBoundObject();
        if (!indexBuffer) {
            MGLOG_W("DrawElementsBaseVertex skipped: no bound ELEMENT_ARRAY_BUFFER");
            return;
        }

        const auto indexData = indexBuffer->GetDataReadOnly();
        if (!indexData || indexData->empty()) {
            MGLOG_W("DrawElementsBaseVertex skipped: ELEMENT_ARRAY_BUFFER has no data");
            return;
        }

        const SizeT byteOffset = reinterpret_cast<SizeT>(indices);
        const SizeT requiredBytes = static_cast<SizeT>(count) * indexSize;
        if (byteOffset + requiredBytes > indexBuffer->GetSize()) {
            MGLOG_W("DrawElementsBaseVertex skipped: index range out of bounds (offset=%zu, size=%zu, buffer=%zu)",
                    byteOffset, requiredBytes, indexBuffer->GetSize());
            return;
        }

        DrawElementPayload payload{};
        payload.drawArray.mode = mode;
        payload.drawArray.first = 0;
        payload.drawArray.count = count;
        const auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        payload.drawArray.program = currentProgram ? currentProgram.get() : nullptr;
        payload.drawArray.vertexArray = vao.get();
        payload.indexType = type;
        payload.indexByteOffset = byteOffset;
        payload.baseVertex = basevertex;

        pVulkanRenderer->DrawElements(payload);
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                                     GLsizei drawcount, const GLint* basevertex) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::MultiDrawElementsBaseVertex called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::MultiDrawElementsBaseVertex called with null GL context");

        if (drawcount < 0) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: drawcount (%d) must be non-negative", drawcount);
            return;
        }
        if (drawcount == 0) {
            return;
        }
        if (!count || !indices || !basevertex) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: count/indices/basevertex pointer is null");
            return;
        }
        if (mode != GL_TRIANGLES) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: primitive mode %u is not supported yet", mode);
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
            MGLOG_W("MultiDrawElementsBaseVertex skipped: index type %u is not supported yet", type);
            return;
        }

        const auto vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: no bound VAO");
            return;
        }

        const auto indexBuffer = vao->GetIndexBufferBindingSlot().GetBoundObject();
        if (!indexBuffer) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: no bound ELEMENT_ARRAY_BUFFER");
            return;
        }

        const auto indexData = indexBuffer->GetDataReadOnly();
        if (!indexData || indexData->empty()) {
            MGLOG_W("MultiDrawElementsBaseVertex skipped: ELEMENT_ARRAY_BUFFER has no data");
            return;
        }

        const auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        Vector<DrawElementPayload> payloads;
        payloads.reserve(static_cast<SizeT>(drawcount));
        for (GLsizei i = 0; i < drawcount; ++i) {
            if (count[i] < 0) {
                MGLOG_W("MultiDrawElementsBaseVertex skipped: count[%d] (%d) must be non-negative", i, count[i]);
                return;
            }
            if (count[i] == 0) {
                continue;
            }

            const SizeT byteOffset = reinterpret_cast<SizeT>(indices[i]);
            const SizeT requiredBytes = static_cast<SizeT>(count[i]) * indexSize;
            if (byteOffset + requiredBytes > indexBuffer->GetSize()) {
                MGLOG_W("MultiDrawElementsBaseVertex skipped: draw[%d] index range out of bounds (offset=%zu, "
                        "size=%zu, buffer=%zu)",
                        i, byteOffset, requiredBytes, indexBuffer->GetSize());
                return;
            }

            DrawElementPayload payload{};
            payload.drawArray.mode = mode;
            payload.drawArray.first = 0;
            payload.drawArray.count = count[i];
            payload.drawArray.program = currentProgram ? currentProgram.get() : nullptr;
            payload.drawArray.vertexArray = vao.get();
            payload.indexType = type;
            payload.indexByteOffset = byteOffset;
            payload.baseVertex = basevertex[i];
            payloads.push_back(payload);
        }

        if (payloads.empty()) {
            return;
        }
        pVulkanRenderer->MultiDrawElements(payloads);
    }
    void MultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride) {}
    void MultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {}
    void DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                     const void* indices, GLint basevertex) {}
    void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {}
    void DrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                     GLsizei instancecount, GLint basevertex, GLuint baseinstance) {}
    void DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                         GLsizei instancecount, GLint basevertex) {}
    void DrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                           GLsizei instancecount, GLuint baseinstance) {}
    void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {}
    void DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) {}
    void DrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                         GLuint baseinstance) {}
    void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {}
    void DrawArraysIndirect(GLenum mode, const void* indirect) {}
    void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width,
                        GLsizei height, GLint border) {}
    void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width,
                           GLsizei height) {}
    void GenerateMipmap(GLenum target) {}
    void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {}
    void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {}

    void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::MultiDrawElements called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::MultiDrawElements called with null GL context");

        if (drawcount < 0) {
            MGLOG_W("MultiDrawElements skipped: drawcount (%d) must be non-negative", drawcount);
            return;
        }
        if (drawcount == 0) {
            return;
        }
        if (!count || !indices) {
            MGLOG_W("MultiDrawElements skipped: count/indices pointer is null");
            return;
        }
        if (mode != GL_TRIANGLES) {
            MGLOG_W("MultiDrawElements skipped: primitive mode %u is not supported yet", mode);
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
            MGLOG_W("MultiDrawElements skipped: index type %u is not supported yet", type);
            return;
        }

        const auto vao = MG_State::pGLContext->GetBoundVertexArray();
        if (!vao) {
            MGLOG_W("MultiDrawElements skipped: no bound VAO");
            return;
        }

        const auto indexBuffer = vao->GetIndexBufferBindingSlot().GetBoundObject();
        if (!indexBuffer) {
            MGLOG_W("MultiDrawElements skipped: no bound ELEMENT_ARRAY_BUFFER");
            return;
        }

        const auto indexData = indexBuffer->GetDataReadOnly();
        if (!indexData || indexData->empty()) {
            MGLOG_W("MultiDrawElements skipped: ELEMENT_ARRAY_BUFFER has no data");
            return;
        }

        const auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        Vector<DrawElementPayload> payloads;
        payloads.reserve(static_cast<SizeT>(drawcount));
        for (GLsizei i = 0; i < drawcount; ++i) {
            if (count[i] < 0) {
                MGLOG_W("MultiDrawElements skipped: count[%d] (%d) must be non-negative", i, count[i]);
                return;
            }
            if (count[i] == 0) {
                continue;
            }

            const auto byteOffset = reinterpret_cast<SizeT>(indices[i]);
            const SizeT requiredBytes = static_cast<SizeT>(count[i]) * indexSize;
            if (byteOffset + requiredBytes > indexBuffer->GetSize()) {
                MGLOG_W("MultiDrawElements skipped: draw[%d] index range out of bounds (offset=%zu, size=%zu, "
                        "buffer=%zu)",
                        i, byteOffset, requiredBytes, indexBuffer->GetSize());
                return;
            }

            DrawElementPayload payload{};
            payload.drawArray.mode = mode;
            payload.drawArray.first = 0;
            payload.drawArray.count = count[i];
            payload.drawArray.program = currentProgram ? currentProgram.get() : nullptr;
            payload.drawArray.vertexArray = vao.get();
            payload.indexType = type;
            payload.indexByteOffset = byteOffset;
            payloads.push_back(payload);
        }

        if (payloads.empty()) {
            return;
        }
        pVulkanRenderer->MultiDrawElements(payloads);
    }

    void Clear(GLbitfield mask) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::Clear called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::Clear called with null GL context");

        const auto& drawFboSlot = MG_State::pGLContext->GetFramebufferBindingSlot(FramebufferTarget::Draw);
        const auto drawFbo = drawFboSlot.GetBoundObject();
        const auto defaultFboInfo = MG_Impl::GLImpl::FramebufferImpl::pDefaultFramebufferInfo;
        const auto defaultFbo = defaultFboInfo ? defaultFboInfo->defaultFBO : nullptr;
        const Bool isDefaultFboTarget = (drawFbo == defaultFbo) || (drawFbo == nullptr && defaultFbo != nullptr);
        Uint drawFboExternalIndex = 0;
        if (drawFbo) {
            drawFboExternalIndex = drawFbo->GetExternalIndex();
        } else if (defaultFbo) {
            drawFboExternalIndex = defaultFbo->GetExternalIndex();
        }

        const auto& clearColor = MG_State::pGLContext->GetClearColor();
        const auto clearDepth = MG_State::pGLContext->GetClearDepth();
        const auto clearStencil = static_cast<Uint32>(MG_State::pGLContext->GetClearStencil());
        pVulkanRenderer->QueueClearRequest(mask, clearColor, clearDepth, clearStencil,
                                           drawFboExternalIndex,
                                           isDefaultFboTarget);
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

        const auto byteOffset = reinterpret_cast<SizeT>(indices);
        const SizeT requiredBytes = static_cast<SizeT>(count) * indexSize;
        if (byteOffset + requiredBytes > indexBuffer->GetSize()) {
            MGLOG_W("DrawElements skipped: index range out of bounds (offset=%zu, size=%zu, buffer=%zu)", byteOffset,
                    requiredBytes, indexBuffer->GetSize());
            return;
        }

        DrawElementPayload payload{};
        payload.drawArray.mode = mode;
        payload.drawArray.first = 0;
        payload.drawArray.count = count;
        const auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        payload.drawArray.program = currentProgram ? currentProgram.get() : nullptr;
        payload.drawArray.vertexArray = vao.get();
        payload.indexType = type;
        payload.indexByteOffset = byteOffset;

        pVulkanRenderer->DrawElements(payload);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawArrays called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawArrays called with null GL context");

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

        DrawArrayPayload payload{};
        payload.mode = mode;
        payload.first = first;
        payload.count = count;
        const auto currentProgram = MG_State::pGLContext->GetCurrentProgram();
        payload.program = currentProgram ? currentProgram.get() : nullptr;

        const auto vao = MG_State::pGLContext->GetBoundVertexArray();
        payload.vertexArray = vao ? vao.get() : nullptr;

        pVulkanRenderer->DrawArrays(payload);
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
    }

    void Present() {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::Present called with null VulkanRenderer");
        pVulkanRenderer->Present();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
