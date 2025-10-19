#pragma once
#include <Includes.h>
#include <MG_State/GLState/Core.h>

namespace MobileGL::MG_Backend::DirectGLES {
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

        void NormalizePixelFormat(GLenum internalFormat, GLenum* outInternalFormat, GLenum* outType, GLenum* outFormat);
        void GenerateTextureFormatInfo(TextureInternalFormat internalFormat, GLenum* outInternalFormat, GLenum* outType,
                                       GLenum* outFormat);
    } // namespace TextureImpl

    namespace FramebufferImpl {
        class BackendFramebufferBindingProtector {
        public:
            BackendFramebufferBindingProtector(GLenum target);

            ~BackendFramebufferBindingProtector();

        private:
            GLenum m_target;
            GLint m_previousBinding = 0;
        };
    } // namespace FramebufferImpl

    namespace PrgramImpl {
        String ProcessOutColorLocations(const String& glslCode);
        String ForceSupporterOutput(const String& glslCode);
        String removeLayoutBinding(const String& glslCode);
    } // namespace PrgramImpl

    namespace Utils {
        void CheckGLESError();
        GLenum GetBindingQuery(GLenum target, bool isTexture);
    } // namespace Utils
} // namespace MobileGL::MG_Backend::DirectGLES