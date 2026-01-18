// MobileGL - MobileGL/MG_State/GLState/RenderbufferState/RenderbufferState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "RenderbufferState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            RenderbufferState::RenderbufferState() : m_indexGenerator(1024, 1) {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    m_bindingSlots[i] = BindingSlot<RenderbufferObject>(static_cast<RenderbufferTarget>(i));
                }
            }

            SharedPtr<RenderbufferObject> RenderbufferState::GetRenderbufferObject(Uint index) {
                auto it = m_renderbufferObjects.find(index);
                if (it != m_renderbufferObjects.end()) {
                    return it->second;
                }
                return nullptr;
            }

            Vector<Uint> RenderbufferState::GenerateNames(Uint number) {
                Vector<Uint> buffers(number);
                m_indexGenerator.Generate(number, buffers.data());
                return buffers;
            }

            SharedPtr<RenderbufferObject> RenderbufferState::CreateRenderbufferObject(Uint index) {
                auto bufferObject = MakeShared<RenderbufferObject>(index);
                m_renderbufferObjects[index] = bufferObject;
                return bufferObject;
            }

            BindingSlot<RenderbufferObject>& RenderbufferState::GetBindingSlot(RenderbufferTarget target) {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    if (m_bindingSlots[i].GetTarget() == target) {
                        return m_bindingSlots[i];
                    }
                }
                MOBILEGL_ASSERT(false, "Invalid RenderbufferTarget enum value: %d", static_cast<int>(target));
                return m_bindingSlots[0];
            }

            void RenderbufferState::MarkRenderbufferObjectForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    auto it = m_renderbufferObjects.find(index);
                    if (it != m_renderbufferObjects.end()) {
                        for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                            if (m_bindingSlots[i].GetBoundObject() == it->second) {
                                m_bindingSlots[i].Bind(nullptr);
                            }
                        }
                        m_renderbufferObjects.erase(it);
                    }
                    m_indexGenerator.Delete(index);
                }
            }

            Bool RenderbufferState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            Bool RenderbufferState::ValidateRenderbufferObject(Uint index) const {
                return m_renderbufferObjects.find(index) != m_renderbufferObjects.end();
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
