#include "Core.h"

namespace MobileGL {
    namespace MG_State {
        void Init() {
            MGLOG_D("Initializing MobileGL State...");
            pGLContext = new MG_State::GLState::GLContext();
        }

        namespace GLState {
            // Error
            void GLContext::RecordError(ErrorCode code, SharedPtr<ErrorInfo> info) {
                m_errorState.RecordError(code, info);
            }

            Bool GLContext::HasGLError() const {
                return m_errorState.HasGLError();
            }

            Optional<const Error> GLContext::PeekGLError() const {
                return m_errorState.PeekGLError();
            }

            Optional<Error> GLContext::PopGLError() {
                return m_errorState.PopGLError();
            }

            Bool GLContext::HasNonGLError() const {
                return m_errorState.HasNonGLError();
            }

            Optional<const Error> GLContext::PeekNonGLError() const {
                return m_errorState.PeekNonGLError();
            }

            Optional<Error> GLContext::PopNonGLError() {
                return m_errorState.PopNonGLError();
            }

            void GLContext::ClearErrors() {
                m_errorState.Clear();
            }

            // Buffer
            Vector<Uint> GLContext::GenBufferNames(Uint number) {
                return m_bufferState.GenerateNames(number);
            }

            SharedPtr<BufferObject> GLContext::GetBufferObject(Uint index) {
                return m_bufferState.GetBufferObject(index);
            }

            BindingSlot<BufferObject>& GLContext::GetBufferBindingSlot(BufferTarget target) {
                if (target == BufferTarget::Index) {
                    const auto& vao = m_vertexArrayState.GetBoundVertexArray();
                    if (vao) {
                        return vao->GetIndexBufferBindingSlot();
                    } else {
                        assert(false);
                        static BindingSlot<BufferObject> defaultSlot(BufferTarget::Index);
                        return defaultSlot;
                    }
                }

                return m_bufferState.GetBindingSlot(target);
            }

            BindingSlotRange1D<BufferObject>& GLContext::GetBufferBindingPoint(BufferTarget target, Uint index) {
                return m_bufferState.GetBindingPoint(target, index);
            }

            SharedPtr<BufferObject> GLContext::CreateBufferObject(Uint index) {
                return m_bufferState.CreateBufferObject(index);
            }

            void GLContext::MarkBufferObjectForDeletion(Uint index) {
                if (ValidateBufferObject(index)) {
                    auto bufferObject = m_bufferState.GetBufferObject(index);
                    auto& vaos = m_vertexArrayState.GetAllVertexArrays();
                    for (SizeT i = 0; i < vaos.size(); ++i) {
                        auto vao = vaos[i];
                        if (vao == nullptr) continue;

                        if (vao->GetIndexBufferBindingSlot().GetBoundObject() == bufferObject) {
                            vao->GetIndexBufferBindingSlot().Bind(nullptr);
                        }
                        for (SizeT j = 0; j < VertexArrayObject::MAX_VERTEX_ATTRIBS; ++j) {
                            if (vao->GetAttribute(j).Buffer == bufferObject) {
                                vao->BindAttributeBuffer(j, nullptr);
                            }
                        }
                    }
                }

                m_bufferState.MarkBufferObjectForDeletion(index);
            }

            Bool GLContext::ValidateBufferName(Uint index) const {
                return m_bufferState.ValidateName(index);
            }

            Bool GLContext::ValidateBufferObject(Uint index) const {
                return m_bufferState.ValidateBufferObject(index);
            }

            // VertexArray
            Vector<Uint> GLContext::GenVertexArrayNames(Uint number) {
                return m_vertexArrayState.GenerateNames(number);
            }

            SharedPtr<VertexArrayObject> GLContext::GetVertexArrayObject(Uint index) {
                return m_vertexArrayState.GetVertexArrayObject(index);
            }

            void GLContext::BindVertexArray(Uint index) {
                m_vertexArrayState.Bind(index);
            }

            SharedPtr<VertexArrayObject> GLContext::CreateVertexArrayObject(Uint index) {
                return m_vertexArrayState.CreateVertexArrayObject(index);
            }

            void GLContext::MarkVertexArrayForDeletion(Uint index) {
                m_vertexArrayState.MarkVertexArrayForDeletion(index);
            }

            Bool GLContext::ValidateVertexArrayName(Uint index) const {
                return m_vertexArrayState.ValidateName(index);
            }

            Bool GLContext::ValidateVertexArrayObject(Uint index) const {
                return m_vertexArrayState.ValidateVertexArrayObject(index);
            }

            SharedPtr<VertexArrayObject> GLContext::GetBoundVertexArray() {
                return m_vertexArrayState.GetBoundVertexArray();
            }

            // Texture
            Vector<Uint> GLContext::GenTextureNames(Uint number) {
                return m_textureState.GenerateNames(number);
            }

            SharedPtr<ITextureObject> GLContext::GetTextureObject(Uint index) {
                return m_textureState.GetTextureObject(index);
            }

            SharedPtr<ITextureObject> GLContext::CreateTextureObject(Uint index, TextureTarget target) {
                return m_textureState.CreateTextureObject(index, target);
            }

            void GLContext::MarkTextureObjectForDeletion(Uint index) {
                m_textureState.MarkTextureObjectForDeletion(index);
            }

            TextureUnit& GLContext::GetTextureUnitObject(Int unit) {
                return m_textureState.GetUnitObject(unit);
            }

            Bool GLContext::ValidateTextureName(Uint index) const {
                return m_textureState.ValidateName(index);
            }

            Bool GLContext::ValidateTextureObject(Uint index) const {
                return m_textureState.ValidateTextureObject(index);
            }

            Int GLContext::GetActiveTextureUnit() const {
                return m_textureState.GetActiveTextureUnit();
            }

            void GLContext::SetActiveTextureUnit(Int unit) {
                m_textureState.SetActiveTextureUnit(unit);
            }

            // Program
            Uint GLContext::CreateProgram() {
                return m_programState.CreateProgram();
            }

            Uint GLContext::CreateShader(const ShaderStage stage) {
                return m_programState.CreateShader(stage);
            }

            void GLContext::MarkProgramForDeletion(const Uint index) {
                return m_programState.MarkProgramObjectForDeletion(index);
            }

            void GLContext::MarkShaderForDeletion(const Uint index) {
                return m_programState.MarkShaderObjectForDeletion(index);
            }

            Bool GLContext::ValidateProgramName(const Uint index) const {
                return m_programState.ValidateProgramObject(index);
            }

            Bool GLContext::ValidateShaderName(const Uint index) const {
                return m_programState.ValidateShaderObject(index);
            }

            SharedPtr<ProgramObject> GLContext::GetProgramObject(const Uint index) {
                return m_programState.GetProgramObject(index);
            }

            SharedPtr<ShaderObject> GLContext::GetShaderObject(const Uint index) {
                return m_programState.GetShaderObject(index);
            }

            void GLContext::UseProgram(Uint program) {
                return m_programState.UseProgram(program);
            }

            SharedPtr<ProgramObject> GLContext::GetCurrentProgram() {
                return m_programState.GetCurrentProgram();
            }

            // RenderState
            void GLContext::SetViewport(IntVec4 viewport) {
                m_renderState.SetViewport(viewport);
            }

            const IntVec4& GLContext::GetViewport() const {
                return m_renderState.GetViewport();
            }

            void GLContext::SetCapability(CapabilityInput cap, Bool enabled) {
                m_renderState.SetCapability(cap, enabled);
            }

            Bool GLContext::IsCapabilityEnabled(CapabilityInput cap) const {
                return m_renderState.IsCapabilityEnabled(cap);
            }

            void GLContext::SetBlendFunc(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                         BlendFactor dstAlpha) {
                m_renderState.SetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);
            }

            void GLContext::GetBlendFunc(BlendFactor& srcRGB, BlendFactor& dstRGB, BlendFactor& srcAlpha,
                                         BlendFactor& dstAlpha) const {
                m_renderState.GetBlendFunc(srcRGB, dstRGB, srcAlpha, dstAlpha);
            }

            void GLContext::SetDepthFunc(DepthTestFunc func) {
                m_renderState.SetDepthFunc(func);
            }

            DepthTestFunc GLContext::GetDepthFunc() const {
                return m_renderState.GetDepthFunc();
            }

            void GLContext::SetDepthMask(Bool flag) {
                m_renderState.SetDepthMask(flag);
            }

            Bool GLContext::GetDepthMask() const {
                return m_renderState.GetDepthMask();
            }

            void GLContext::SetColorMask(BoolVec4 mask) {
                m_renderState.SetColorMask(mask);
            }

            const BoolVec4 GLContext::GetColorMask() const {
                return m_renderState.GetColorMask();
            }

            void GLContext::SetClearColor(FloatVec4 color) {
                m_renderState.SetClearColor(color);
            }

            const FloatVec4& GLContext::GetClearColor() const {
                return m_renderState.GetClearColor();
            }

            void GLContext::SetClearDepth(Float depth) {
                m_renderState.SetClearDepth(depth);
            }

            Float GLContext::GetClearDepth() const {
                return m_renderState.GetClearDepth();
            }

            void GLContext::SetPixelStoreParam(PixelStoreParam param, Int value) {
                m_renderState.SetPixelStoreParam(param, value);
            }

            Int GLContext::GetPixelStoreParam(PixelStoreParam param) const {
                return m_renderState.GetPixelStoreParam(param);
            }

            PixelStoreParameters GLContext::GetPixelStoreParameters(Bool isUnpack) const {
                return m_renderState.GetPixelStoreParameters(isUnpack);
            }

            void GLContext::SetCullFaceMode(CullFaceMode mode) {
                m_renderState.SetCullFaceMode(mode);
            }

            CullFaceMode GLContext::GetCullFaceMode() const {
                return m_renderState.GetCullFaceMode();
            }

            void GLContext::SetScissorBox(IntVec4 box) {
                m_renderState.SetScissorBox(box);
            }

            const IntVec4& GLContext::GetScissorBox() const {
                return m_renderState.GetScissorBox();
            }

            // Framebuffer
            Vector<Uint> GLContext::GenFramebufferNames(Uint number) {
                return m_framebufferState.GenerateNames(number);
            }

            SharedPtr<FramebufferObject> GLContext::GetFramebufferObject(Uint index) {
                return m_framebufferState.GetFramebufferObject(index);
            }

            BindingSlot<FramebufferObject>& GLContext::GetFramebufferBindingSlot(FramebufferTarget target) {
                return m_framebufferState.GetBindingSlot(target);
            }

            SharedPtr<FramebufferObject> GLContext::CreateFramebufferObject(Uint index) {
                return m_framebufferState.CreateFramebufferObject(index);
            }

            void GLContext::MarkFramebufferObjectForDeletion(Uint index) {
                m_framebufferState.MarkFramebufferObjectForDeletion(index);
            }

            Bool GLContext::ValidateFramebufferName(Uint index) const {
                return m_framebufferState.ValidateName(index);
            }

            Bool GLContext::ValidateFramebufferObject(Uint index) const {
                return m_framebufferState.ValidateFramebufferObject(index);
            }

        } // namespace GLState

        GLState::GLContext* pGLContext;
    } // namespace MG_State
} // namespace MobileGL