#pragma once
#include <Includes.h>
#include "BufferObject.h"
#include <MG_Util/Miscellany/IndexGenerator.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            constexpr const auto GlobalBufferTargets =
                ToArray(BufferTarget::Vertex, BufferTarget::Uniform, BufferTarget::CopyRead, BufferTarget::CopyWrite,
                        BufferTarget::PixelPack, BufferTarget::PixelUnpack, BufferTarget::Query, BufferTarget::Texture,
                        BufferTarget::TransformFeedback, BufferTarget::AtomicCounter, BufferTarget::DispatchIndirect,
                        BufferTarget::DrawIndirect, BufferTarget::ShaderStorage);

            class BufferState {
            public:
                BufferState();

                SharedPtr<BufferObject> GetBufferObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<BufferObject> CreateBufferObject(Uint index);
                BindingSlot<BufferObject>& GetBindingSlot(BufferTarget target);
                void MarkBufferObjectForDeletion(Uint index);
                bool ValidateName(Uint index) const;
                bool ValidateBufferObject(Uint index) const;

            private:
                UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
                IndexGenerator<Uint> m_indexGenerator;
                Array<BindingSlot<BufferObject>, GlobalBufferTargets.size()> m_bindingSlots;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL