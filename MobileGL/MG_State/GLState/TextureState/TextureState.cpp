#include "TextureState.h"
#include "Defines.h"
#include "TextureEnum.h"
#include "TextureObject.h"
#include "TextureObject1D.h"
#include "TextureObject2D.h"
#include "TextureObject3D.h"
#include "TextureObject2DCube.h"
#include "TextureObjectBuffer.h"
#include "TextureObjectStubs.h"

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
                    textureObject = MakeShared<TextureObject1D>(index);
                    break;
                case TextureTarget::TextureCubeMap:
                    textureObject = MakeShared<TextureObject2DCube>(index);
                    break;
                case TextureTarget::Texture2D:
                    textureObject = MakeShared<TextureObject2D>(index);
                    break;
                case TextureTarget::Texture3D:
                    textureObject = MakeShared<TextureObject3D>(index);
                    break;
                case TextureTarget::TextureBuffer:
                    textureObject = MakeShared<TextureObjectBuffer>(index);
                    break;

                    // These texture types are stubbed:
                case TextureTarget::TextureRectangle:
                    textureObject = MakeShared<TextureObjectRectangle>(index);
                    break;
                case TextureTarget::Texture2DMultisample:
                    textureObject = MakeShared<TextureObject2DMultisample>(index);
                    break;
                case TextureTarget::Texture1DArray:
                    textureObject = MakeShared<TextureObject1DArray>(index);
                    break;
                case TextureTarget::Texture2DArray:
                    textureObject = MakeShared<TextureObject2DArray>(index);
                    break;
                case TextureTarget::TextureCubeMapArray:
                    textureObject = MakeShared<TextureObjectCubeMapArray>(index);
                    break;
                case TextureTarget::Texture2DMultisampleArray:
                    textureObject = MakeShared<TextureObject2DMultisampleArray>(index);
                    break;
                default:
                    MOBILEGL_ASSERT(false, "Unimplemented texture type when creating texture object!: %d", (int)target);
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
                MOBILEGL_ASSERT(unit >= 0 && unit < MAX_TEXTURE_IMAGE_UNITS, "Texture unit is out of range: %d > %d",
                                unit, MAX_TEXTURE_IMAGE_UNITS - 1);
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
