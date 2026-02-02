// MobileGL - MobileGL/MG_State/GLState/VertexArrayState/VertexArrayObject.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "../BufferState/BufferObject.h"
#include "MG_Util/Types.h"

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

            struct VertexAttributeVersion {
                Uint16 FormatVersion = 0;
                Uint16 BufferVersion = 0;
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

                Uint GetExternalIndex() const;

                void SetAttributeDivisor(Uint index, Uint divisor);
                Uint GetAttributeDivisor(Uint index) const;

                const VertexAttributeVersion& GetAttributeVersion(Uint index) const;
                const Array<VertexAttributeVersion, MAX_VERTEX_ATTRIBS>& GetAllAttributeVersions() const;

            private:
                void BumpAttributeFormatVersion(Uint index);
                void BumpAttributeBufferVersion(Uint index);

                const Uint m_externalIndex = 0;
                Array<VertexAttribute, MAX_VERTEX_ATTRIBS> m_attributes;
                Array<VertexAttributeVersion, MAX_VERTEX_ATTRIBS> m_attributeVersions; // format version, buffer version
                BindingSlot<BufferObject> m_indexBufferBindingSlot;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
