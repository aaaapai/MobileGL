#include "RenderbufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderbufferObject::RenderbufferObject(Uint externalIndex) : m_externalIndex(externalIndex) {}

            void RenderbufferObject::Storage(Int internalFormat, Int width, Int height) {
                m_internalFormat = internalFormat;
                m_width = width;
                m_height = height;
                m_allocated = true;
            }

            Int RenderbufferObject::GetWidth() const {
                return m_width;
            }

            Int RenderbufferObject::GetHeight() const {
                return m_height;
            }

            Int RenderbufferObject::GetInternalFormat() const {
                return m_internalFormat;
            }

            Bool RenderbufferObject::IsAllocated() const {
                return m_allocated;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL