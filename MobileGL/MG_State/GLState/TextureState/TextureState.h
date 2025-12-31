// MobileGL - MobileGL/MG_State/GLState/TextureState/TextureState.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_Util/Miscellany/IndexGenerator.h>
#include "MG_State/GLState/TextureState/TextureObject.h"
#include "MG_Util/Types.h"
#include "TextureUnit.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            class TextureState {
            public:
                static constexpr int MAX_TEXTURE_IMAGE_UNITS = 32;

                TextureState();
                Vector<Uint> GenerateNames(Uint number);
                SharedPtr<ITextureObject> CreateTextureObject(Uint index, TextureTarget target);
                SharedPtr<ITextureObject> GetTextureObject(Uint index);
                TextureUnit& GetUnitObject(Int unit);
                Int GetActiveTextureUnit() const;
                void SetActiveTextureUnit(Int unit);
                void MarkTextureObjectForDeletion(Uint index);
                Bool ValidateName(Uint index) const;
                Bool ValidateTextureObject(Uint index) const;

            private:
                Int m_activeTextureUnit = 0;
                Array<TextureUnit, MAX_TEXTURE_IMAGE_UNITS> m_textureUnits;
                IndexGenerator<Uint> m_indexGenerator;
                UnorderedMap<GLuint, SharedPtr<ITextureObject>> m_textureObjects;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
