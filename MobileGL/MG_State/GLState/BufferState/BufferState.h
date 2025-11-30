#pragma once
#include <Includes.h>
#include <MG_Util/Miscellany/IndexGenerator.h>
#include "BufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            constexpr const auto GlobalBufferTargets =
                ToArray(BufferTarget::Vertex, BufferTarget::Uniform, BufferTarget::CopyRead, BufferTarget::CopyWrite,
                        BufferTarget::PixelPack, BufferTarget::PixelUnpack, BufferTarget::Query, BufferTarget::Texture,
                        BufferTarget::TransformFeedback, BufferTarget::AtomicCounter, BufferTarget::DispatchIndirect,
                        BufferTarget::DrawIndirect, BufferTarget::ShaderStorage);
            constexpr const auto BufferBindPointTargets =
                ToArray(BufferTarget::Uniform, BufferTarget::TransformFeedback, BufferTarget::AtomicCounter,
                        BufferTarget::ShaderStorage);

            class BufferState {
            public:
                BufferState();

                SharedPtr<BufferObject> GetBufferObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<BufferObject> CreateBufferObject(Uint index);
                BindingSlot<BufferObject>& GetBindingSlot(BufferTarget target);
                // For glBindBufferBase / glBindBufferRange
                BindingSlotRange1D<BufferObject>& GetBindingPoint(BufferTarget target, Uint index);
                constexpr SizeT GetBindingPointCount(const BufferTarget target) const {
                    auto it = std::find(BufferBindPointTargets.begin(), BufferBindPointTargets.end(), target);
                    auto index = std::distance(BufferBindPointTargets.begin(), it);
                    return m_bufferBindPointTargets[index].size();
                }
                void MarkBufferObjectForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateBufferObject(Uint index) const;

            private:
                UnorderedMap<Uint, SharedPtr<BufferObject>> m_bufferObjects;
                IndexGenerator<Uint> m_indexGenerator;
                Array<BindingSlot<BufferObject>, GlobalBufferTargets.size()> m_bindingSlots;
                // TODO: query the count somewhere globally?
                // For glBindBufferBase / glBindBufferRange
                Array<Array<BindingSlotRange1D<BufferObject>, 16>, BufferBindPointTargets.size()>
                    m_bufferBindPointTargets;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL