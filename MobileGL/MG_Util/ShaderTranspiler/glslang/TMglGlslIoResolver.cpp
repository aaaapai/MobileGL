// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/TMglGlslIoResolver.cpp
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

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
        if (currentStage == EShLangFragment && type.getQualifier().isPipeOutput()) {
            auto it = m_explicitFragOuts.find(name.c_str());
            if (it != m_explicitFragOuts.end()) {
                auto& writableType = ent.symbol->getWritableType();
                writableType.getQualifier().layoutLocation = it->second;
            }
        }
        TDefaultGlslIoResolver::reserverStorageSlot(ent, infoSink);
    }
} // namespace MobileGL