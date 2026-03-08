// MobileGL - MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "VertexArrayObject.h"
#include <MG_Util/Miscellany/IndexGenerator.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class VertexArrayState {
            public:
                VertexArrayState();

                const SharedPtr<VertexArrayObject>& GetVertexArrayObject(Uint index);
                void GenerateNames(Uint number, Vector<Uint>& arrays);
                void Bind(Uint index);
                const SharedPtr<VertexArrayObject>& CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateVertexArrayObject(Uint index) const;
                const SharedPtr<VertexArrayObject>& GetBoundVertexArray();
                Vector<SharedPtr<VertexArrayObject>>& GetAllVertexArrays();

            private:
                Vector<SharedPtr<VertexArrayObject>> m_vertexArrays;
                IndexGenerator<Uint> m_indexGenerator;
                SharedPtr<VertexArrayObject> m_boundVertexArray;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
