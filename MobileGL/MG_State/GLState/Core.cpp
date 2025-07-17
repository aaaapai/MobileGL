#include "../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            Vector<Uint> GLContext::GenBufferNames(Uint number) {
                return bufferState_.GenerateNames(number);
            }

            SharedPtr<BufferObject> GLContext::GetBufferObject(Uint index) {
                return bufferState_.GetBufferObject(index);
            }

            BindingSlot<BufferObject>& GLContext::GetBufferBindingSlot(BufferTarget target) {
                return bufferState_.GetBindingSlot(target);
            }

            SharedPtr<BufferObject> GLContext::CreateBufferObject(Uint index) {
                return bufferState_.CreateBufferObject(index);
            }

        }

        GLState::GLContext* pGLContext;
    }
}