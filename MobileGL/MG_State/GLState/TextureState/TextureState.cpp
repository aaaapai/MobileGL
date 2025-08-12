#include "TextureState.h"
#include "MG_State/GLState/TextureState/TextureObject.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            TextureState::TextureState() : m_indexGenerator(1024, 1) {
                for (int i = 0; i < MAX_TEXTURE_IMAGE_UNITS; ++i) {
                    m_textureUnits[i] = TextureUnit();
                }
            }

            SharedPtr<ITextureObject> TextureState::GetTextureObject(Uint index) {
                auto it = m_textureObjects.find(index);
                if (it != m_textureObjects.end()) {
                    return it->second;
                }
                return nullptr;
            }

            Vector<Uint> TextureState::GenerateNames(Uint number) {
                Vector<Uint> textures(number);
                m_indexGenerator.Generate(number, textures.data());
                return textures;
            }

            SharedPtr<ITextureObject> TextureState::CreateTextureObject(Uint index, TextureTarget target) {
                SharedPtr<ITextureObject> textureObject = nullptr;
                switch (target) {
                case TextureTarget::Texture1D:
                    textureObject = MakeShared<TextureObject1D>();
                    break;
                case TextureTarget::Texture2D:
                    textureObject = MakeShared<TextureObject2D>();
                    break;
                case TextureTarget::Texture3D:
                    textureObject = MakeShared<TextureObject3D>();
                    break;
                default:
                    // TODO: implement more texture types
                    return nullptr;
                }

                m_textureObjects[index] = textureObject;
                return textureObject;
            }

            void TextureState::MarkTextureObjectForDeletion(Uint index) {
                if (m_indexGenerator.IsValid(index)) {
                    auto it = m_textureObjects.find(index);
                    if (it != m_textureObjects.end()) {
                        for (auto& unit : m_textureUnits) {
                            auto& bindingSlots = unit.GetAllBindingSlots();
                            for (SizeT i = 0; i < bindingSlots.size(); ++i) {
                                if (bindingSlots[i].GetBoundObject() == it->second) {
                                    bindingSlots[i].Bind(nullptr);
                                }
                            }
                        }
                        m_textureObjects.erase(it);
                    }
                    m_indexGenerator.Delete(index);
                }
            }

            TextureUnit& TextureState::GetUnitObject(Int unit) {
                if (unit < 0 || unit >= MAX_TEXTURE_IMAGE_UNITS) {
                    THROW_EXCEPTION("Active texture unit is out of range");
                }
                return m_textureUnits[unit];
            }

            Int TextureState::GetActiveTextureUnit() const {
                return m_activeTextureUnit;
            }

            void TextureState::SetActiveTextureUnit(Int unit) {
                m_activeTextureUnit = unit;
            }

            Bool TextureState::ValidateName(Uint index) const {
                return m_indexGenerator.IsValid(index);
            }

            Bool TextureState::ValidateTextureObject(Uint index) const {
                return m_textureObjects.find(index) != m_textureObjects.end();
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL
