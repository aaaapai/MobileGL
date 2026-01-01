// MobileGL - MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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