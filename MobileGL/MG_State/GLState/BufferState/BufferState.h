#pragma once

namespace MobileGL {
	namespace MG_State {
		namespace GLState {
			class BufferState {
			public:
				SharedPtr<BufferObject> GetBufferObjectByIndex(Uint index) {
					auto it = m_bufferObjects.find(index);
					if (it != m_bufferObjects.end()) {
						return it->second;
					}
					return nullptr;
				}

				Vector<Uint> GenerateByNum(Uint number) {
					Vector<Uint> buffers;
					m_indexGenerator.Generate(number, reinterpret_cast<Uint*>(&buffers));
					return buffers;
				}

				SharedPtr<BufferObject> CreateBufferObject(Uint index) {
					auto bufferObject = MakeShared<BufferObject>();
					m_bufferObjects[index] = bufferObject;
					return bufferObject;
				}

			private:
				UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
				IndexGenerator<Uint> m_indexGenerator;
			};
		}
	}
}