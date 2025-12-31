// MobileGL - MobileGL/MG_State/GLState/FramebufferState/FramebufferState.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/Miscellany/IndexGenerator.h>
#include "FramebufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class FramebufferState {
            public:
                FramebufferState();

                // FBO 0 should be created by MG_Backend when initializing the context
                SharedPtr<FramebufferObject> GetFramebufferObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<FramebufferObject> CreateFramebufferObject(Uint index);
                BindingSlot<FramebufferObject>& GetBindingSlot(FramebufferTarget target);
                void MarkFramebufferObjectForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateFramebufferObject(Uint index) const;

            private:
                UnorderedMap<Uint, SharedPtr<FramebufferObject>> m_framebufferObjects;
                IndexGenerator<Uint> m_indexGenerator;
                Array<BindingSlot<FramebufferObject>, static_cast<SizeT>(FramebufferTarget::FramebufferTargetCount)>
                    m_bindingSlots;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL