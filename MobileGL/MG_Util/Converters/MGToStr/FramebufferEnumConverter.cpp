// MobileGL - MobileGL/MG_Util/Converters/MGToStr/FramebufferEnumConverter.cpp
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "FramebufferEnumConverter.h"

namespace MobileGL {
    namespace MG_Util {
        String ConvertFramebufferTargetToString(FramebufferTarget bufferTarget) {
            switch (bufferTarget) {
            case FramebufferTarget::Draw:
                return "Draw";
            case FramebufferTarget::Read:
                return "Read";
            case FramebufferTarget::Unknown:
            default:
                return "Unknown";
            }
        }

        String ConvertFramebufferAttachmentTypeToString(FramebufferAttachmentType attachment) {
            if (attachment >= FramebufferAttachmentType::Color0 && attachment <= FramebufferAttachmentType::Color31) {
                return "Color" + std::to_string(static_cast<SizeT>(attachment) -
                                                static_cast<SizeT>(FramebufferAttachmentType::Color0));
            }

            switch (attachment) {
            case FramebufferAttachmentType::Depth:
                return "Depth";
            case FramebufferAttachmentType::Stencil:
                return "Stencil";
            case FramebufferAttachmentType::Unknown:
            default:
                return "Unknown";
            }
        }

        String ConvertRenderbufferTargetToString(RenderbufferTarget target) {
            switch (target) {
            case RenderbufferTarget::Renderbuffer:
                return "Renderbuffer";
            case RenderbufferTarget::Unknown:
            default:
                return "Unknown";
            }
        }
    } // namespace MG_Util
} // namespace MobileGL