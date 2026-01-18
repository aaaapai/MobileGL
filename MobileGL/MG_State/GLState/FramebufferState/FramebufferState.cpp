// MobileGL - MobileGL/MG_State/GLState/FramebufferState/FramebufferState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "FramebufferState.h"
#include "MG_State/GLState/FramebufferState/FramebufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            FramebufferState::FramebufferState() {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    m_bindingSlots[i] = BindingSlot<FramebufferObject>(static_cast<FramebufferTarget>(i));
                }
            }

            SharedPtr<FramebufferObject> FramebufferState::GetFramebufferObject(Uint index) {
                auto it = m_framebufferObjects.find(index);
                if (it != m_framebufferObjects.end()) {
                    return it->second;
                }
                return nullptr;
            }

            Vector<Uint> FramebufferState::GenerateNames(Uint number) {
                Vector<Uint> buffers(number);
                m_indexGenerator.Generate(number, buffers.data());
                return buffers;
            }

            SharedPtr<FramebufferObject> FramebufferState::CreateFramebufferObject(Uint index) {
                if (index == 0) {
                    if (!m_indexGenerator.IsValid(0)) {
                        m_indexGenerator.Insert(0);
                    } else {
                        return nullptr;
                    }
                }
                auto bufferObject = MakeShared<FramebufferObject>(index);
                m_framebufferObjects[index] = bufferObject;
                return bufferObject;
            }

            BindingSlot<FramebufferObject>& FramebufferState::GetBindingSlot(FramebufferTarget target) {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    if (m_bindingSlots[i].GetTarget() == target) {
                        return m_bindingSlots[i];
                    }
                }
                MOBILEGL_ASSERT(false, "Invalid FramebufferTarget enum value: %d", static_cast<int>(target));
                return m_bindingSlots[0];
            }

            void FramebufferState::MarkFramebufferObjectForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    auto it = m_framebufferObjects.find(index);
                    if (it != m_framebufferObjects.end()) {
                        for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                            if (m_bindingSlots[i].GetBoundObject() == it->second) {
                                m_bindingSlots[i].Bind(nullptr);
                            }
                        }
                        m_framebufferObjects.erase(it);
                    }
                    m_indexGenerator.Delete(index);
                }
            }

            Bool FramebufferState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            Bool FramebufferState::ValidateFramebufferObject(Uint index) const {
                return m_framebufferObjects.find(index) != m_framebufferObjects.end();
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
