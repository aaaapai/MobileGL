//
// Created by Swung 0x48 on 2025/11/10.
//

#include "TMglGlslIoResolver.h"

namespace MobileGL {
    void TMglGlslIoResolver::reserverStorageSlot(glslang::TVarEntryInfo& ent, TInfoSink& infoSink) {
        const glslang::TType& type = ent.symbol->getType();
        const glslang::TString& name = ent.symbol->getAccessName();
        if (currentStage == EShLangVertex && type.getQualifier().isPipeInput()) {
            auto it = m_explicitVertexIns.find(name.c_str());
            if (it != m_explicitVertexIns.end()) {
                auto& writableType = ent.symbol->getWritableType();
                writableType.getQualifier().layoutLocation = it->second;
            }
        }
        TDefaultGlslIoResolver::reserverStorageSlot(ent, infoSink);
    }
}