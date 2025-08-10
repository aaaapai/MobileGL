#include "VertexArrayState.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            VertexArrayState::VertexArrayState() : m_indexGenerator(1024, 1) {}

            SharedPtr<VertexArrayObject> VertexArrayState::GetVertexArrayObject(Uint index) {
                if (index >= m_vertexArrays.size())
                    // FIXME: report a GL error here
                    return nullptr;

                return m_vertexArrays[index];
            }

            Vector<Uint> VertexArrayState::GenerateNames(Uint number) {
                Vector<Uint> arrays(number);
                m_indexGenerator.Generate(number, arrays.data());
                return arrays;
            }

            void VertexArrayState::Bind(Uint index) {
                if (index == 0) {
                    // FIXME: should we make a dummy VAO at index 0?
                    m_boundVertexArray = nullptr;
                    return;
                }

                m_boundVertexArray = GetVertexArrayObject(index);
            }

            SharedPtr<VertexArrayObject> VertexArrayState::CreateVertexArrayObject(Uint index) {
                if (index >= m_vertexArrays.size()) {
                    // power-of-2 reallocation
                    m_vertexArrays.reserve(std::bit_ceil(index + 1));
                    m_vertexArrays.resize(index + 1, nullptr);
                }
                auto vao = m_vertexArrays[index] = MakeShared<VertexArrayObject>();
                return vao;
            }

            void VertexArrayState::MarkVertexArrayForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    if (m_boundVertexArray) {
                        m_boundVertexArray = nullptr;
                    }

                    if (ValidateVertexArrayObject(index)) {
                        m_vertexArrays[index] = nullptr;
                    }

                    m_indexGenerator.Delete(index);
                }
                // FIXME: report GL error here?
            }

            Bool VertexArrayState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            Bool VertexArrayState::ValidateVertexArrayObject(Uint index) const {
                return index < m_vertexArrays.size() && m_vertexArrays[index] != nullptr;
            }

            SharedPtr<VertexArrayObject> VertexArrayState::GetBoundVertexArray() {
                return m_boundVertexArray;
            }

            Vector<SharedPtr<VertexArrayObject>>& VertexArrayState::GetAllVertexArrays() {
                return m_vertexArrays;
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
