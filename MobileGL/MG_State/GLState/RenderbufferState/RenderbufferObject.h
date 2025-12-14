#pragma once
#include <Includes.h>
#include <MG_Util/Math/VectorTypes.h>
#include <MG_State/GLState/TextureState/TextureEnum.h>

namespace MobileGL {
    enum class RenderbufferTarget {
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

                void SetInternalFormat(TextureInternalFormat format);
                void AllocateStorage(IntVec2 size);
                Int GetWidth() const;
                Int GetHeight() const;
                TextureInternalFormat GetInternalFormat() const;
                Bool IsAllocated() const;
                const ComponentSizes& GetComponentSizes() const;
                Int GetRedSize() const;
                Int GetGreenSize() const;
                Int GetBlueSize() const;
                Int GetAlphaSize() const;
                Int GetDepthSize() const;
                Int GetStencilSize() const;
                Int GetSamples() const;

            private:
                Uint m_externalIndex = 0;
                TextureInternalFormat m_internalFormat;
                Int m_width = 0;
                Int m_height = 0;
                Int m_samples = 0; // TODO: multisampling support
                Bool m_allocated = false;
                ComponentSizes m_componentSizes;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
