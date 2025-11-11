//
// Created by Swung 0x48 on 2025/11/10.
//

#include "TMglGlslIoResolver.h"

namespace MobileGL {
    void TMglGlslIoResolver::reserverStorageSlot(glslang::TVarEntryInfo& ent, TInfoSink& infoSink) {
        const glslang::TType& type = ent.symbol->getType();
        const glslang::TString& name = ent.symbol->getAccessName();
        if (currentStage == EShLangVertex && type.getQualifier().isPipeInput()) {
            auto it = m_explicitAttribLocations.find(name.c_str());
            if (it != m_explicitAttribLocations.end()) {
                auto& writableType = ent.symbol->getWritableType();
                writableType.getQualifier().layoutLocation = it->second;
            }
        }
        TDefaultGlslIoResolver::reserverStorageSlot(ent, infoSink);
    }
}