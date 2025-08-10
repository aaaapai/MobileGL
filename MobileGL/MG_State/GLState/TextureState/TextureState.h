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
                TextureUnit& GetUnitObject();
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
