#pragma once
#include <Includes.h>
#include "MG_Util/Types.h"
#include "ErrorState/Error.h"
#include "BufferState/BufferState.h"
#include "ProgramState/ProgramState.h"
#include "TextureState/TextureState.h"
#include "FramebufferState/FramebufferState.h"
#include "VertexArrayState/VertexArrayState.h"
#include "MG_State/GLState/RenderState/RenderState.h"

namespace MobileGL {
    namespace MG_State {
        void Init();

        namespace GLState {
            class GLContext {
            public:
                GLContext() = default;

                // Error
                void RecordError(ErrorCode code, SharedPtr<ErrorInfo> info = nullptr);
                Bool HasGLError() const;
                Optional<const Error> PeekGLError() const;
                Optional<Error> PopGLError();
                Bool HasNonGLError() const;
                Optional<const Error> PeekNonGLError() const;
                Optional<Error> PopNonGLError();
                void ClearErrors();

                // Buffer
                Vector<Uint> GenBufferNames(Uint number);
                SharedPtr<BufferObject> GetBufferObject(Uint index);
                BindingSlot<BufferObject>& GetBufferBindingSlot(BufferTarget target);
                SharedPtr<BufferObject> CreateBufferObject(Uint index);
                void MarkBufferObjectForDeletion(Uint index);
                Bool ValidateBufferName(Uint index) const;
                Bool ValidateBufferObject(Uint index) const;

                // VertexArray
                Vector<Uint> GenVertexArrayNames(Uint number);
                SharedPtr<VertexArrayObject> GetVertexArrayObject(Uint index);
                void BindVertexArray(Uint index);
                SharedPtr<VertexArrayObject> CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                Bool ValidateVertexArrayName(Uint index) const;
                Bool ValidateVertexArrayObject(Uint index) const;
                SharedPtr<VertexArrayObject> GetBoundVertexArray();

                // Texture
                Vector<Uint> GenTextureNames(Uint number);
                SharedPtr<ITextureObject> GetTextureObject(Uint index);
                SharedPtr<ITextureObject> CreateTextureObject(Uint index, TextureTarget target);
                void MarkTextureObjectForDeletion(Uint index);
                TextureUnit& GetTextureUnitObject(Int unit);
                Bool ValidateTextureName(Uint index) const;
                Bool ValidateTextureObject(Uint index) const;
                Int GetActiveTextureUnit() const;
                void SetActiveTextureUnit(Int unit);

                // Program
                Uint CreateProgram();
                Uint CreateShader(ShaderStage stage);
                void MarkProgramForDeletion(Uint index);
                void MarkShaderForDeletion(Uint index);
                Bool ValidateProgramName(Uint index) const;
                Bool ValidateShaderName(Uint index) const;
                SharedPtr<ProgramObject> GetProgramObject(Uint index);
                SharedPtr<ShaderObject> GetShaderObject(Uint index);
                void UseProgram(Uint program);
                SharedPtr<ProgramObject> GetCurrentProgram();

                // RenderState
                void SetViewport(IntVec4 viewport); // x, y, width, height
                const IntVec4& GetViewport() const; // x, y, width, height
                void SetCapability(CapabilityInput cap, Bool enabled);
                Bool IsCapabilityEnabled(CapabilityInput cap) const;
                void SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha);
                void GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                  BlendFactor& dstAlpha) const;
                void SetDepthFunc(DepthTestFunc func);
                DepthTestFunc GetDepthFunc() const;
                void SetDepthMask(Bool flag);
                Bool GetDepthMask() const;
                void SetColorMask(BoolVec4 mask);
                const BoolVec4 GetColorMask() const;
                void SetClearColor(FloatVec4 color);
                const FloatVec4& GetClearColor() const;
                void SetClearDepth(Float depth);
                Float GetClearDepth() const;
                void SetPixelStoreParam(PixelStoreParam param, Int value);
                Int GetPixelStoreParam(PixelStoreParam param) const;
                PixelStoreParameters GetPixelStoreParameters(Bool isUnpack) const;
                void SetCullFaceMode(CullFaceMode mode);
                CullFaceMode GetCullFaceMode() const;
                void SetScissorBox(IntVec4 box);      // x, y, width, height
                const IntVec4& GetScissorBox() const; // x, y, width, height

                // Framebuffer
                Vector<Uint> GenFramebufferNames(Uint number);
                SharedPtr<FramebufferObject> GetFramebufferObject(Uint index);
                BindingSlot<FramebufferObject>& GetFramebufferBindingSlot(FramebufferTarget target);
                SharedPtr<FramebufferObject> CreateFramebufferObject(Uint index);
                void MarkFramebufferObjectForDeletion(Uint index);
                Bool ValidateFramebufferName(Uint index) const;
                Bool ValidateFramebufferObject(Uint index) const;

            private:
                // Error
                ErrorState m_errorState;

                // Buffer
                BufferState m_bufferState;

                // VertexArray
                VertexArrayState m_vertexArrayState;

                // Texture
                TextureState m_textureState;

                // Program
                ProgramState m_programState;

                // RenderState
                RenderState m_renderState;

                // Framebuffer
                FramebufferState m_framebufferState;
            };
        } // namespace GLState

        extern GLState::GLContext* pGLContext;
    } // namespace MG_State
} // namespace MobileGL