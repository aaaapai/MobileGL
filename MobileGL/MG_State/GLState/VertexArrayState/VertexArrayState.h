// MobileGL - MobileGL/MG_State/GLState/VertexArrayState/VertexArrayState.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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

                SharedPtr<VertexArrayObject> GetVertexArrayObject(Uint index);
                Vector<Uint> GenerateNames(Uint number);
                void Bind(Uint index);
                SharedPtr<VertexArrayObject> CreateVertexArrayObject(Uint index);
                void MarkVertexArrayForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateVertexArrayObject(Uint index) const;
                SharedPtr<VertexArrayObject> GetBoundVertexArray();
                Vector<SharedPtr<VertexArrayObject>>& GetAllVertexArrays();

            private:
                Vector<SharedPtr<VertexArrayObject>> m_vertexArrays;
                IndexGenerator<Uint> m_indexGenerator;
                SharedPtr<VertexArrayObject> m_boundVertexArray;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
