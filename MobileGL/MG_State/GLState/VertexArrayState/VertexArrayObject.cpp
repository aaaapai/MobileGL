#include "../../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            VertexArrayObject::VertexArrayObject() {
                for (auto& attr : m_attributes) {
                    attr.Enabled = false;
                    attr.Size = 4;
                    attr.Type = DataType::Float32;
                    attr.Normalized = false;
                    attr.Stride = 0;
                    attr.Offset = 0;
                    attr.Buffer = nullptr;
                }
            }

            void VertexArrayObject::EnableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Enabled = true;
            }

            void VertexArrayObject::DisableAttribute(Uint index) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Enabled = false;
            }

            bool VertexArrayObject::IsAttributeEnabled(Uint index) const {
                if (index >= MAX_VERTEX_ATTRIBS) return false;
                return m_attributes[index].Enabled;
            }

            void VertexArrayObject::SetAttributeFormat(Uint index, int size, DataType type,
                bool normalized, int stride, SizeT offset, bool isInteger) {
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
            }

            void VertexArrayObject::BindAttributeBuffer(Uint index,
                const SharedPtr<BufferObject>& buffer) {
                if (index >= MAX_VERTEX_ATTRIBS) return;
                m_attributes[index].Buffer = buffer;
            }

            void VertexArrayObject::BindElementBuffer(const SharedPtr<BufferObject>& buffer) {
                m_elementBuffer = buffer;
            }

            SharedPtr<BufferObject> VertexArrayObject::GetElementBuffer() const {
                return m_elementBuffer;
            }

            const VertexAttribute& VertexArrayObject::GetAttribute(Uint index) const {
                static VertexAttribute emptyAttr;
                if (index >= MAX_VERTEX_ATTRIBS) return emptyAttr;
                return m_attributes[index];
            }
        }
    }
}
