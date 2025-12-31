// MobileGL - MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "../BufferState/BufferObject.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            struct VertexAttribute {
                Bool Enabled = false;
                int Size = 4;
                DataType Type = DataType::Float32;
                Bool Normalized = false;
                int Stride = 0;
                SizeT Offset = 0;
                Bool IsInteger = false;
                Uint Divisor = 0;
                SharedPtr<BufferObject> Buffer;
            };

            class VertexArrayObject {
            public:
                static constexpr int MAX_VERTEX_ATTRIBS = 16;

                VertexArrayObject(Uint externIndex);

                void EnableAttribute(Uint index);
                void DisableAttribute(Uint index);
                Bool IsAttributeEnabled(Uint index) const;

                void SetAttributeFormat(Uint index, int size, DataType type, Bool normalized, int stride, SizeT offset,
                                        Bool isInteger);

                void BindAttributeBuffer(Uint index, const SharedPtr<BufferObject>& buffer);

                BindingSlot<BufferObject>& GetIndexBufferBindingSlot();

                const VertexAttribute& GetAttribute(Uint index) const;
                const Array<VertexAttribute, MAX_VERTEX_ATTRIBS>& GetAllAttributes() const;

                const Vector<Uint>& GetDirtyAttributeIndices() const;
                void ClearDirtyAttributes();
                Uint GetExternalIndex() const;

                void SetAttributeDivisor(Uint index, Uint divisor);
                Uint GetAttributeDivisor(Uint index) const;

            private:
                void MarkAttributeDirty(Uint index);

                const Uint m_externalIndex = 0;
                Array<VertexAttribute, MAX_VERTEX_ATTRIBS> m_attributes;
                Vector<Uint> m_dirtyAttributes;
                BindingSlot<BufferObject> m_indexBufferBindingSlot;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
