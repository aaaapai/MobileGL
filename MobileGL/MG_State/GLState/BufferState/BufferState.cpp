// MobileGL - MobileGL/MG_State/GLState/BufferState/BufferState.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "BufferState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            BufferState::BufferState() : m_indexGenerator(1024, 1) {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    m_bindingSlots[i] = BindingSlot<BufferObject>(GlobalBufferTargets[i]);
                }
            }

            SharedPtr<BufferObject> BufferState::GetBufferObject(Uint index) {
                auto it = m_bufferObjects.find(index);
                if (it != m_bufferObjects.end()) {
                    return it->second;
                }
                return nullptr;
            }

            Vector<Uint> BufferState::GenerateNames(Uint number) {
                Vector<Uint> buffers(number);
                m_indexGenerator.Generate(number, buffers.data());
                return buffers;
            }

            SharedPtr<BufferObject> BufferState::CreateBufferObject(Uint index) {
                auto bufferObject = MakeShared<BufferObject>(index);
                m_bufferObjects[index] = bufferObject;
                return bufferObject;
            }

            BindingSlot<BufferObject>& BufferState::GetBindingSlot(BufferTarget target) {
                for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                    if (m_bindingSlots[i].GetTarget() == target) {
                        return m_bindingSlots[i];
                    }
                }
                MOBILEGL_ASSERT(false, "Invalid BufferTarget enum value: %d", static_cast<int>(target));
                return m_bindingSlots[0];
            }

            void BufferState::MarkBufferObjectForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    auto it = m_bufferObjects.find(index);
                    if (it != m_bufferObjects.end()) {
                        for (SizeT i = 0; i < m_bindingSlots.size(); ++i) {
                            if (m_bindingSlots[i].GetBoundObject() == it->second) {
                                m_bindingSlots[i].Bind(nullptr);
                            }
                        }
                        m_bufferObjects.erase(it);
                    }
                    m_indexGenerator.Delete(index);
                }
            }

            Bool BufferState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            Bool BufferState::ValidateBufferObject(Uint index) const {
                return m_bufferObjects.find(index) != m_bufferObjects.end();
            }

            BindingSlotRange1D<BufferObject>& BufferState::GetBindingPoint(BufferTarget target, Uint index) {
                for (SizeT i = 0; i < BufferBindPointTargets.size(); ++i) {
                    if (BufferBindPointTargets[i] == target) {
                        return m_bufferBindPointTargets[i][index];
                    }
                }
                MOBILEGL_ASSERT(false, "Invalid BufferTarget enum value for binding point: %d",
                                static_cast<int>(target));
                return m_bufferBindPointTargets[0][index];
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
