#pragma once

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class VertexArrayState {
            public:
                VertexArrayState();

                SharedPtr<VertexArrayObject> GetVertexArrayObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                void Bind(Uint index);
                SharedPtr<VertexArrayObject> CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                bool ValidateName(Uint index) const;
                bool ValidateVertexArrayObject(Uint index) const;
                SharedPtr<VertexArrayObject> GetBoundVertexArray();
                Vector<SharedPtr<VertexArrayObject>> GetAllVertexArrays();

            private:
                UnorderedMap<Uint, SharedPtr<VertexArrayObject>> m_vertexArrays;
                IndexGenerator<Uint> m_indexGenerator;
                SharedPtr<VertexArrayObject> m_boundVertexArray;
            };
        }
    }
}
