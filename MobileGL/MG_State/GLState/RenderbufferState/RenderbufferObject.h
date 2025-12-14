#pragma once
#include <Includes.h>

namespace MobileGL {
    enum class RenderbufferTarget { // fucky stuff in GL spec
        Renderbuffer,
        RenderbufferTargetCount,
        Unknown = -1
    };

    namespace MG_State {
        namespace GLState {
            class RenderbufferObject {
            public:
                using TargetEnum = RenderbufferTarget;

                RenderbufferObject(Uint externalIndex);

                void Storage(Int internalFormat, Int width, Int height);
                Int GetWidth() const;
                Int GetHeight() const;
                Int GetInternalFormat() const;
                Bool IsAllocated() const;

            private:
                Uint m_externalIndex = 0;
                Int m_internalFormat = 0;
                Int m_width = 0;
                Int m_height = 0;
                Bool m_allocated = false;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
