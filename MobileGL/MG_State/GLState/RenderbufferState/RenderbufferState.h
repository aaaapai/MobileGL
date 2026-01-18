// MobileGL - MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/Miscellany/IndexGenerator.h>
#include "RenderbufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class RenderbufferState {
            public:
                RenderbufferState();

                SharedPtr<RenderbufferObject> GetRenderbufferObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<RenderbufferObject> CreateRenderbufferObject(Uint index);
                BindingSlot<RenderbufferObject>& GetBindingSlot(RenderbufferTarget target);
                void MarkRenderbufferObjectForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateRenderbufferObject(Uint index) const;

            private:
                UnorderedMap<Uint, SharedPtr<RenderbufferObject>> m_renderbufferObjects;
                IndexGenerator<Uint> m_indexGenerator;
                Array<BindingSlot<RenderbufferObject>, static_cast<SizeT>(RenderbufferTarget::RenderbufferTargetCount)>
                    m_bindingSlots;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL