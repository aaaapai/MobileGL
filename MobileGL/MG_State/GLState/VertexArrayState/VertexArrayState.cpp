#include "../../../Includes.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            VertexArrayState::VertexArrayState()
                : m_indexGenerator(1024, 1) {
            }

            SharedPtr<VertexArrayObject> VertexArrayState::GetVertexArrayObject(Uint index) {
                auto it = m_vertexArrays.find(index);
                if (it != m_vertexArrays.end()) {
                    return it->second;
                }
                return nullptr;
            }

            Vector<Uint> VertexArrayState::GenerateNames(Uint number) {
                Vector<Uint> arrays(number);
                m_indexGenerator.Generate(number, arrays.data());
                return arrays;
            }

            void VertexArrayState::Bind(Uint index) {
                if (index == 0) {
                    m_boundVertexArray = nullptr;
                    return;
                }

                auto it = m_vertexArrays.find(index);
                if (it != m_vertexArrays.end()) {
                    m_boundVertexArray = it->second;
                }
                else {
                    m_boundVertexArray = nullptr;
                }
            }

            SharedPtr<VertexArrayObject> VertexArrayState::CreateVertexArrayObject(Uint index) {
                auto vao = MakeShared<VertexArrayObject>();
                m_vertexArrays[index] = vao;
                return vao;
            }

            void VertexArrayState::MarkVertexArrayForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    if (m_boundVertexArray && m_vertexArrays.find(index) != m_vertexArrays.end() &&
                        m_vertexArrays[index] == m_boundVertexArray) {
                        m_boundVertexArray = nullptr;
                    }

                    m_vertexArrays.erase(index);
                    m_indexGenerator.Delete(index);
                }
            }

            bool VertexArrayState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            bool VertexArrayState::ValidateVertexArrayObject(Uint index) const {
                return m_vertexArrays.find(index) != m_vertexArrays.end();
            }

            SharedPtr<VertexArrayObject> VertexArrayState::GetBoundVertexArray() {
                return m_boundVertexArray;
            }
        }
    }
}
