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

                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<SamplerObject> GetSamplerObject(Uint index);
                SharedPtr<SamplerObject> CreateSamplerObject(Uint index);
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
