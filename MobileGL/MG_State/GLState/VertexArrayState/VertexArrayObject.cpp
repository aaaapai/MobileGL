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

                    MarkAttributeDirty(index);
                }
            }

            void VertexArrayObject::EnableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Enabled = true;
                MarkAttributeDirty(index);
            }

            void VertexArrayObject::DisableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Enabled = false;
                MarkAttributeDirty(index);
            }

            Bool VertexArrayObject::IsAttributeEnabled(Uint index) const {
                if (index >= MAX_VERTEX_ATTRIBS) return false;
                return m_attributes[index].Enabled;
            }

            void VertexArrayObject::SetAttributeFormat(Uint index, int size, DataType type, Bool normalized, int stride,
                                                       SizeT offset, Bool isInteger) {
                if (index >= MAX_VERTEX_ATTRIBS) return;

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

                MarkAttributeDirty(index);
            }

            void VertexArrayObject::BindAttributeBuffer(Uint index, const SharedPtr<BufferObject>& buffer) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Buffer = buffer;
                MarkAttributeDirty(index);
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

            void VertexArrayObject::MarkAttributeDirty(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                if (std::find(m_dirtyAttributes.begin(), m_dirtyAttributes.end(), index) != m_dirtyAttributes.end()) {
                    return;
                }
                m_dirtyAttributes.push_back(index);
            }

            const Vector<Uint>& VertexArrayObject::GetDirtyAttributeIndices() const {
                return m_dirtyAttributes;
            }

            void VertexArrayObject::ClearDirtyAttributes() {
                m_dirtyAttributes.clear();
            }

            Uint VertexArrayObject::GetExternalIndex() const {
                return m_externalIndex;
            }

            void VertexArrayObject::SetAttributeDivisor(Uint index, Uint divisor) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                if (m_attributes[index].Divisor == divisor) return;
                m_attributes[index].Divisor = divisor;
                MarkAttributeDirty(index);
            }

            Uint VertexArrayObject::GetAttributeDivisor(Uint index) const {
                if (index >= MAX_VERTEX_ATTRIBS) return 0;
                return m_attributes[index].Divisor;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
