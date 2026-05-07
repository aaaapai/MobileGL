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

    void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::ClearBufferfi called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::ClearBufferfi called with null GL context");
        pVulkanRenderer->ClearBufferfi(buffer, drawbuffer, depth, stencil);
    }

    void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::ClearBufferfv called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::ClearBufferfv called with null GL context");
        pVulkanRenderer->ClearBufferfv(buffer, drawbuffer, value);
    }

    void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::ClearBufferuiv called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::ClearBufferuiv called with null GL context");
        pVulkanRenderer->ClearBufferuiv(buffer, drawbuffer, value);
    }

    void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::ClearBufferiv called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::ClearBufferiv called with null GL context");
        pVulkanRenderer->ClearBufferiv(buffer, drawbuffer, value);
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
                        GLsizei height, GLint border) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::CopyTexImage2D called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::CopyTexImage2D called with null GL context");
        pVulkanRenderer->CopyTexSubImage2D(target, level, 0, 0, x, y, width, height);
    }
    void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width,
                           GLsizei height) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::CopyTexSubImage2D called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::CopyTexSubImage2D called with null GL context");
        pVulkanRenderer->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }
    void GenerateMipmap(GLenum target) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::GenerateMipmap called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::GenerateMipmap called with null GL context");
        pVulkanRenderer->GenerateMipmap(target);
    }
    void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {}
    void GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {}

    void Clear(GLbitfield mask) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::Clear called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::Clear called with null GL context");
        pVulkanRenderer->Clear(mask);
    }

    void DrawArrays(GLenum mode, GLint first, GLsizei count) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawArrays called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawArrays called with null GL context");

        DrawCmd payload{};
        payload.mode = mode;
        payload.params.firstVertex = first;
        payload.params.vertexCount = count;

        pVulkanRenderer->DrawArrays(payload);
    }

    void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawElements called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawElements called with null GL context");

        DrawIndexedCmd payload{};
        payload.mode = mode;
        payload.indexBufferView.indexType = type;
        payload.indexBufferView.indexByteOffset = reinterpret_cast<SizeT>(indices);
        payload.indexBufferView.indexByteSize = count * MG_Util::GetGLTypeSize(type);
        payload.params.indexCount = count;
        payload.params.instanceCount = 1;

        pVulkanRenderer->DrawElements(payload);
    }

    void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                           GLsizei drawcount) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::MultiDrawElements called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::MultiDrawElements called with null GL context");

        // Vector<DrawElementCmd> cmds;
        // cmds.reserve(static_cast<SizeT>(drawcount));
        // for (GLsizei i = 0; i < drawcount; ++i) {
        //     if (count[i] == 0) {
        //         continue;
        //     }
        //
        //     DrawElementCmd payload{};
        //     payload.mode = mode;
        //     payload.firstVertex = 0;
        //     payload.indexCount = count[i];
        //     payload.indexType = type;
        //     payload.indexByteOffset = reinterpret_cast<SizeT>(indices[i]);
        //     cmds.push_back(payload);
        // }
        //
        // if (cmds.empty()) {
        //     return;
        // }
        // pVulkanRenderer->MultiDrawElements(cmds);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::DrawElementsBaseVertex called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::DrawElementsBaseVertex called with null GL context");
        DrawIndexedCmd payload{};
        payload.mode = mode;
        payload.indexBufferView.indexType = type;
        payload.indexBufferView.indexByteOffset = reinterpret_cast<SizeT>(indices);
        payload.indexBufferView.indexByteSize = count * MG_Util::GetGLTypeSize(type);
        payload.params.indexCount = count;
        payload.params.instanceCount = 1;
        payload.params.firstIndex = 0;
        payload.params.vertexOffset = basevertex;
        payload.params.firstInstance = 0;
        pVulkanRenderer->DrawElements(payload);
    }

    void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices,
                                     GLsizei drawcount, const GLint* basevertex) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::MultiDrawElements called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::MultiDrawElements called with null GL context");
        MultiDrawIndexedCmd payload{};
        payload.mode = mode;
        payload.indexBufferView.indexType = type;

        // TODO: allocate draw cmd buf elsewhere
        static Vector<DrawIndexedCmdParam> params;
        params.clear();
        params.resize(drawcount);

        for (GLsizei i = 0; i < drawcount; ++i) {
            if (count[i] == 0) {
                continue;
            }

            // TODO: this index view needs a redesign, now there's a lotta redundant uploads

            payload.indexBufferView.indexByteOffset = 0;
            payload.indexBufferView.indexByteSize =
                    std::max(reinterpret_cast<SizeT>(indices[i]) + count[i] * MG_Util::GetGLTypeSize(type),
                             payload.indexBufferView.indexByteSize);

            auto& param = params[i];

            param.indexCount = count[i];
            param.instanceCount = 1;
            param.firstIndex = reinterpret_cast<SizeT>(indices[i]) / MG_Util::GetGLTypeSize(type);
            param.vertexOffset = basevertex[i];
            param.firstInstance = 0;
        }
        payload.drawCount = drawcount;
        payload.pParams = params.data();
        pVulkanRenderer->MultiDrawElements(payload);
    }

    void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1,
                         GLint dstY1, GLbitfield mask, GLenum filter) {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::BlitFramebuffer called with null VulkanRenderer");
        MOBILEGL_ASSERT(MG_State::pGLContext, "DirectVulkan::BlitFramebuffer called with null GL context");
        pVulkanRenderer->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    void Present() {
        MOBILEGL_ASSERT(pVulkanRenderer, "DirectVulkan::Present called with null VulkanRenderer");
        pVulkanRenderer->Present();
    }
} // namespace MobileGL::MG_Backend::DirectVulkan
