// MobileGL - MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VertexArrayObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            VertexArrayObject::VertexArrayObject(Uint externIndex) : m_externalIndex(externIndex) {
                for (int index = 0; index < MAX_VERTEX_ATTRIBS; ++index) {
                    auto& attr = m_attributes[index];
                    attr.Enabled = false;
                    attr.Size = 4;
                    attr.Type = DataType::Float32;
                    attr.Normalized = false;
                    attr.Stride = 0;
                    attr.Offset = 0;
                    attr.Buffer = nullptr;

                    BumpAttributeFormatVersion(index);
                }
            }

            void VertexArrayObject::EnableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;

                if (!m_attributes[index].Enabled) return;

                m_attributes[index].Enabled = true;
                BumpAttributeSwitchVersion(index);
            }

            void VertexArrayObject::DisableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;

                if (!m_attributes[index].Enabled) return;

                m_attributes[index].Enabled = false;
                BumpAttributeSwitchVersion(index);
            }

            Bool VertexArrayObject::IsAttributeEnabled(Uint index) const {
                if (index >= MAX_VERTEX_ATTRIBS) return false;
                return m_attributes[index].Enabled;
            }

            void VertexArrayObject::SetAttributeFormat(Uint index, int size, DataType type, Bool normalized, int stride,
                                                       SizeT offset, Bool isInteger) {
                if (index >= MAX_VERTEX_ATTRIBS) return;

                if (m_attributes[index].Size == size && m_attributes[index].Type == type &&
                    m_attributes[index].Normalized == normalized && m_attributes[index].Stride == stride &&
                    m_attributes[index].Offset == offset && m_attributes[index].IsInteger == isInteger) {
                    return;
                }

                if (size < 1 || size > 4) {
                    return;
                }

                auto& attr = m_attributes[index];
                attr.Size = size;
                attr.Type = type;
                attr.Normalized = normalized;
                attr.Stride = stride;
                attr.Offset = offset;
                attr.IsInteger = isInteger;

                BumpAttributeFormatVersion(index);
            }

            void VertexArrayObject::BindAttributeBuffer(Uint index, const SharedPtr<BufferObject>& buffer) {
                if (index >= MAX_VERTEX_ATTRIBS) return;

                if (m_attributes[index].Buffer == buffer) return;

                m_attributes[index].Buffer = buffer;
                BumpAttributeBufferVersion(index);
            }

            BindingSlot<BufferObject>& VertexArrayObject::GetIndexBufferBindingSlot() {
                return m_indexBufferBindingSlot;
            }

            const VertexAttribute& VertexArrayObject::GetAttribute(Uint index) const {
                static VertexAttribute emptyAttr;
                if (index >= MAX_VERTEX_ATTRIBS) return emptyAttr;
                return m_attributes[index];
            }

            const Array<VertexAttribute, VertexArrayObject::MAX_VERTEX_ATTRIBS>& VertexArrayObject::GetAllAttributes()
                const {
                return m_attributes;
            }

            Uint VertexArrayObject::GetExternalIndex() const {
                return m_externalIndex;
            }

            void VertexArrayObject::SetAttributeDivisor(Uint index, Uint divisor) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                if (m_attributes[index].Divisor == divisor) return;
                m_attributes[index].Divisor = divisor;
                BumpAttributeFormatVersion(index);
            }

            Uint VertexArrayObject::GetAttributeDivisor(Uint index) const {
                if (index >= MAX_VERTEX_ATTRIBS) return 0;
                return m_attributes[index].Divisor;
            }

            void VertexArrayObject::BumpAttributeFormatVersion(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                ++m_attributeVersions[index].FormatVersion;
            }

            void VertexArrayObject::BumpAttributeBufferVersion(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                ++m_attributeVersions[index].BufferVersion;
            }

            void VertexArrayObject::BumpAttributeSwitchVersion(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                ++m_attributeVersions[index].SwitchVersion;
            }

            const VertexAttributeVersion& VertexArrayObject::GetAttributeVersion(Uint index) const {
                static VertexAttributeVersion emptyVersion;
                if (index >= MAX_VERTEX_ATTRIBS) return emptyVersion;
                return m_attributeVersions[index];
            }

            const Array<VertexAttributeVersion, VertexArrayObject::MAX_VERTEX_ATTRIBS>& VertexArrayObject::
                GetAllAttributeVersions() const {
                return m_attributeVersions;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
