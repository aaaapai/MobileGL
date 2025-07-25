#pragma once

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            struct VertexAttribute {
                bool Enabled = false;
                int Size = 4;
                DataType Type = DataType::Float32;
                bool Normalized = false;
                int Stride = 0;
                SizeT Offset = 0;
                SharedPtr<BufferObject> Buffer;
            };

            class VertexArrayObject {
            public:
                static constexpr int MAX_VERTEX_ATTRIBS = 16;

                VertexArrayObject();

                void EnableAttribute(Uint index);
                void DisableAttribute(Uint index);
                bool IsAttributeEnabled(Uint index) const;

                void SetAttributeFormat(Uint index, int size, DataType type, bool normalized, int stride, SizeT offset);

                void BindAttributeBuffer(Uint index, const SharedPtr<BufferObject>& buffer);

                void BindElementBuffer(const SharedPtr<BufferObject>& buffer);
                SharedPtr<BufferObject> GetElementBuffer() const;

                const VertexAttribute& GetAttribute(Uint index) const;

            private:
                Array<VertexAttribute, MAX_VERTEX_ATTRIBS> m_attributes;
                SharedPtr<BufferObject> m_elementBuffer;
            };
        }
    }
}
