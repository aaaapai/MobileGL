// MobileGL - MobileGL/MG_State/GLState/RenderbufferState/RenderbufferObject.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "RenderbufferObject.h"
#include <MG_Util/Metrics/TextureMetrics.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderbufferObject::RenderbufferObject(Uint externalIndex) : m_externalIndex(externalIndex) {}

            Int RenderbufferObject::GetWidth() const {
                return m_width;
            }

            Int RenderbufferObject::GetHeight() const {
                return m_height;
            }

            TextureInternalFormat RenderbufferObject::GetInternalFormat() const {
                return m_internalFormat;
            }

            Bool RenderbufferObject::IsAllocated() const {
                return m_allocated;
            }

            Int RenderbufferObject::GetRedSize() const {
                return m_componentSizes.Red;
            }

            Int RenderbufferObject::GetGreenSize() const {
                return m_componentSizes.Green;
            }

            Int RenderbufferObject::GetBlueSize() const {
                return m_componentSizes.Blue;
            }

            Int RenderbufferObject::GetAlphaSize() const {
                return m_componentSizes.Alpha;
            }

            Int RenderbufferObject::GetDepthSize() const {
                return m_componentSizes.Depth;
            }

            Int RenderbufferObject::GetStencilSize() const {
                return m_componentSizes.Stencil;
            }

            Int RenderbufferObject::GetSamples() const {
                return m_samples;
            }

            const ComponentSizes& RenderbufferObject::GetComponentSizes() const {
                return m_componentSizes;
            }

            void RenderbufferObject::SetInternalFormat(TextureInternalFormat format) {
                m_internalFormat = format;
                m_componentSizes = MG_Util::GetComponentSizesForInternalFormat(format);
            }

            void RenderbufferObject::AllocateStorage(IntVec2 size) {
                m_width = size.x();
                m_height = size.y();
                m_allocated = true;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
