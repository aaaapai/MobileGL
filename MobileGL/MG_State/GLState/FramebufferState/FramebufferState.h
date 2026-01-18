// MobileGL - MobileGL/MG_State/GLState/FramebufferState/FramebufferState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
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