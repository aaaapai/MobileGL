// MobileGL - MobileGL/MG_State/GLState/Core.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "ErrorState/Error.h"
#include "BufferState/BufferState.h"
#include "MG_State/GLState/RenderbufferState/RenderbufferObject.h"
#include "MG_State/GLState/RenderbufferState/RenderbufferState.h"
#include "RenderState/RenderState.h"
#include "ProgramState/ProgramState.h"
#include "SamplerState/SamplerState.h"
#include "TextureState/TextureState.h"
#include "FramebufferState/FramebufferState.h"
#include "VertexArrayState/VertexArrayState.h"
#include "RenderbufferState/RenderbufferState.h"

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
                BindingSlotRange1D<BufferObject>& GetBufferBindingPoint(BufferTarget target, Uint index);
                constexpr SizeT GetBufferBindingPointCount(BufferTarget target) const {
                    return m_bufferState.GetBindingPointCount(target);
                }
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

                // Sampler
                Vector<Uint> GenSamplerNames(Uint number);
                SharedPtr<SamplerObject> GetSamplerObject(Uint index);
                SharedPtr<SamplerObject> CreateSamplerObject(Uint index);
                void MarkSamplerObjectForDeletion(Uint index);
                Bool ValidateSamplerName(Uint index) const;
                Bool ValidateSamplerObject(Uint index) const;

                // Renderbuffer
                Vector<Uint> GenRenderbufferNames(Uint number);
                SharedPtr<RenderbufferObject> GetRenderbufferObject(Uint index);
                BindingSlot<RenderbufferObject>& GetRenderbufferBindingSlot(RenderbufferTarget target);
                SharedPtr<RenderbufferObject> CreateRenderbufferObject(Uint index);
                void MarkRenderbufferObjectForDeletion(Uint index);
                Bool ValidateRenderbufferName(Uint index) const;
                Bool ValidateRenderbufferObject(Uint index) const;

            private:
                // State Components
                ErrorState m_errorState;
                BufferState m_bufferState;
                VertexArrayState m_vertexArrayState;
                TextureState m_textureState;
                ProgramState m_programState;
                RenderState m_renderState;
                FramebufferState m_framebufferState;
                SamplerState m_samplerState;
                RenderbufferState m_renderbufferState;
            };
        } // namespace GLState

        extern GLState::GLContext* pGLContext;
    } // namespace MG_State
} // namespace MobileGL