// MobileGL - MobileGL/MG_State/GLState/SamplerState/SamplerState.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "SamplerObject.h"
#include <MG_Util/Miscellany/IndexGenerator.h>
#include <Includes.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class SamplerState {
            public:
                SamplerState();

                void GenerateNames(Uint number, Vector<Uint>& samplers);
                const SharedPtr<SamplerObject>& GetSamplerObject(Uint index);
                const SharedPtr<SamplerObject>& CreateSamplerObject(Uint index);
                void MarkSamplerObjectForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateSamplerObject(Uint index) const;

            private:
                UnorderedMap<Uint, SharedPtr<SamplerObject>> m_samplerObjects;
                IndexGenerator<Uint> m_indexGenerator;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
