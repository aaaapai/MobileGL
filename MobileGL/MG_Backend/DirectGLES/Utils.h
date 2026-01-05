// MobileGL - MobileGL/MG_Backend/DirectGLES/Utils.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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

        void GenerateTextureFormatInfo(TextureInternalFormat internalFormat, GLenum* outInternalFormat, GLenum* outType,
                                       GLenum* outFormat);
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
