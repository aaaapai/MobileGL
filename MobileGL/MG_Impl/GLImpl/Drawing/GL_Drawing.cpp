// MobileGL - MobileGL/MG_Impl/GLImpl/Drawing/GL_Drawing.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "GL_Drawing.h"
#include <Config.h>
#include <MG_State/GLState/Core.h>
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
#include <MG_Backend/DirectGLES/DirectGLES.h>
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_VULKAN
#include <MG_Backend/DirectVulkan/TmpImpl.h>
#include <MG_Backend/DirectVulkan/DirectVulkan.h>
#endif
#include <MG_Util/BackendLoaders/OpenGL/Loader.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        void Clear_Backend(GLbitfield mask) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::Clear(mask);
#elif MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_VULKAN
            MG_Backend::DirectVulkan::Clear(mask);
#endif
        }

        void DrawElements_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElements(mode, count, type, indices);
#elif MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_VULKAN
            MG_Backend::DirectVulkan::DrawElements(mode, count, type, indices);
#endif
        }

        void MultiDrawElements_Backend(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                                       GLsizei drawcount) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawElements(mode, count, type, indices, drawcount);
#endif
        }

        void MultiDrawElementsBaseVertex_Backend(GLenum mode, const GLsizei* count, GLenum type,
                                                 const void* const* indices, GLsizei drawcount,
                                                 const GLint* basevertex) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
#endif
        }

        void DrawArrays_Backend(GLenum mode, GLint first, GLsizei count) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawArrays(mode, first, count);
#endif
        }

        void DrawElementsBaseVertex_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                            GLint basevertex) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsBaseVertex(mode, count, type, indices, basevertex);
#elif MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_VULKAN
            MG_Backend::DirectVulkan::TmpImpl::DrawElementsBaseVertex(mode, count, type, indices, basevertex);
#endif
        }

        void MultiDrawElementsIndirect_Backend(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount,
                                               GLsizei stride) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
#endif
        }

        void MultiDrawArraysIndirect_Backend(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::MultiDrawArraysIndirect(mode, indirect, drawcount, stride);
#endif
        }

        void DrawRangeElementsBaseVertex_Backend(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                                 const void* indices, GLint basevertex) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
#endif
        }

        void DrawRangeElements_Backend(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                       const void* indices) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawRangeElements(mode, start, end, count, type, indices);
#endif
        }

        void DrawElementsInstancedBaseVertexBaseInstance_Backend(GLenum mode, GLsizei count, GLenum type,
                                                                 const void* indices, GLsizei instancecount,
                                                                 GLint basevertex, GLuint baseinstance) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsInstancedBaseVertexBaseInstance(
                mode, count, type, indices, instancecount, basevertex, baseinstance);
#endif
        }

        void DrawElementsInstancedBaseVertex_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                     GLsizei instancecount, GLint basevertex) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount,
                                                                    basevertex);
#endif
        }

        void DrawElementsInstancedBaseInstance_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                       GLsizei instancecount, GLuint baseinstance) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount,
                                                                      baseinstance);
#endif
        }

        void DrawElementsInstanced_Backend(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                           GLsizei instancecount) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsInstanced(mode, count, type, indices, instancecount);
#endif
        }

        void DrawElementsIndirect_Backend(GLenum mode, GLenum type, const void* indirect) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawElementsIndirect(mode, type, indirect);
#endif
        }
        void DrawArraysInstancedBaseInstance_Backend(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                                     GLuint baseinstance) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
#endif
        }

        void DrawArraysInstanced_Backend(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawArraysInstanced(mode, first, count, instancecount);
#endif
        }

        void DrawArraysIndirect_Backend(GLenum mode, const void* indirect) {
#ifdef TRACY_ENABLE
            ZoneScopedC(TRACY_ZONECOLOR_BACKEND);
#endif
#if MOBILEGL_BACKEND == MOBILEGL_BACKEND_TYPE_DIRECT_GLES
            MG_Backend::DirectGLES::DrawArraysIndirect(mode, indirect);
#endif
        }

        /* @INSERTION_POINT:FUNCTION_IMPLEMENTATION@ */
        void MultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount,
                                       GLsizei stride) {
            MultiDrawElementsIndirect_Backend(mode, type, indirect, drawcount, stride);
        }

        void MultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride) {
            MultiDrawArraysIndirect_Backend(mode, indirect, drawcount, stride);
        }

        void DrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type,
                                         const void* indices, GLint basevertex) {
            DrawRangeElementsBaseVertex_Backend(mode, start, end, count, type, indices, basevertex);
        }

        void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices) {
            DrawRangeElements_Backend(mode, start, end, count, type, indices);
        }

        void DrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                                         GLsizei instancecount, GLint basevertex, GLuint baseinstance) {
            DrawElementsInstancedBaseVertexBaseInstance_Backend(mode, count, type, indices, instancecount, basevertex,
                                                                baseinstance);
        }

        void DrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                             GLsizei instancecount, GLint basevertex) {
            DrawElementsInstancedBaseVertex_Backend(mode, count, type, indices, instancecount, basevertex);
        }

        void DrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                               GLsizei instancecount, GLuint baseinstance) {
            DrawElementsInstancedBaseInstance_Backend(mode, count, type, indices, instancecount, baseinstance);
        }

        void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices,
                                   GLsizei instancecount) {
            DrawElementsInstanced_Backend(mode, count, type, indices, instancecount);
        }

        void DrawElementsIndirect(GLenum mode, GLenum type, const void* indirect) {
            DrawElementsIndirect_Backend(mode, type, indirect);
        }

        void DrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount,
                                             GLuint baseinstance) {
            DrawArraysInstancedBaseInstance_Backend(mode, first, count, instancecount, baseinstance);
        }

        void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
            DrawArraysInstanced_Backend(mode, first, count, instancecount);
        }

        void DrawArraysIndirect(GLenum mode, const void* indirect) {
            DrawArraysIndirect_Backend(mode, indirect);
        }

        void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) {
            DrawElementsBaseVertex_Backend(mode, count, type, indices, basevertex);
        }

        void DrawArrays(GLenum mode, GLint first, GLsizei count) {
            DrawArrays_Backend(mode, first, count);
        }

        void MultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                               GLsizei drawcount) {
            MultiDrawElements_Backend(mode, count, type, indices, drawcount);
        }

        void MultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices,
                                         GLsizei drawcount, const GLint* basevertex) {
            MultiDrawElementsBaseVertex_Backend(mode, count, type, indices, drawcount, basevertex);
        }

        void Clear(GLbitfield mask) {
            Clear_Backend(mask);
        }

        void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
            DrawElements_Backend(mode, count, type, indices);
        }

    } // namespace MG_Impl::GLImpl
} // namespace MobileGL
