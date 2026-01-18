// MobileGL - MobileGL/MG_Backend/DirectGLES/Utils.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Backend::DirectGLES {
    namespace DebugImpl {
        class ErrorLopper {
        public:
            void Loop(std::function<void(GLenum)>);
            void Clear();
            ErrorLopper();
            ~ErrorLopper();
        };

        class OpenGLScopeMarker {
        public:
            explicit OpenGLScopeMarker(String scopeName);
            ~OpenGLScopeMarker();
        };
    } // namespace DebugImpl

    namespace BufferImpl {
        class BackendBufferBindingProtector {
        public:
            BackendBufferBindingProtector(GLenum target);

            ~BackendBufferBindingProtector();

        private:
            GLenum m_target;
            GLint m_previousBinding = 0;
        };
    } // namespace BufferImpl

    namespace VertexArrayImpl {
        GLenum GetBindingQuery(GLenum target, bool isTexture);

        class BackendVertexArrayBindingProtector {
        public:
            BackendVertexArrayBindingProtector();

            ~BackendVertexArrayBindingProtector();

        private:
            GLint m_previousBinding = 0;
        };
    } // namespace VertexArrayImpl

    namespace TextureImpl {
        class BackendTextureBindingProtector {
        public:
            BackendTextureBindingProtector(GLenum target);

            ~BackendTextureBindingProtector();

        private:
            GLenum m_target;
            GLint m_previousBinding = 0;
        };

        void GenerateTextureFormatInfo(TextureInternalFormat internalFormat, GLenum* outInternalFormat,
                                       GLenum* outFormat, GLenum* outType);
    } // namespace TextureImpl

    namespace FramebufferImpl {
        class BackendFramebufferBindingProtector {
        public:
            BackendFramebufferBindingProtector(GLenum target);

            ~BackendFramebufferBindingProtector();

            static GLuint GetTempFBO(FramebufferTarget target);
            static void BindTempFBO(FramebufferTarget target);

        private:
            GLenum m_target;
            GLint m_previousBinding = 0;
            inline static GLuint s_tempReadFBO = 0;
            inline static GLuint s_tempDrawFBO = 0;
        };
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        String ProcessOutColorLocations(const String& glslCode);
        String ForceSupporterOutput(const String& glslCode);
        String RemoveLayoutBinding(const String& glslCode);
    } // namespace PrgramImpl

    namespace Utils {
        void CheckGLESError();
        GLenum GetBindingQuery(GLenum target, bool isTexture);
    } // namespace Utils
} // namespace MobileGL::MG_Backend::DirectGLES
