//
// Created by Swung 0x48 on 2025/11/10.
//

#pragma once

#include <vector>
#include <unordered_map>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/Types.h>
#include <glslang/Include/intermediate.h>
#include <glslang/MachineIndependent/iomapper.h>
#include "TVarEntryInfo.h"
#include "MG_Util/Types.h"

namespace MobileGL {
    class TMglGlslIoResolver: public glslang::TDefaultGlslIoResolver {
    public:
        using ExplicitVarSlotMap = UnorderedMap<String, Uint>;
        TMglGlslIoResolver(const glslang::TIntermediate& intermediate, const ExplicitVarSlotMap& vertexIns, const ExplicitVarSlotMap& fragOuts):
            TDefaultGlslIoResolver(intermediate), m_explicitVertexIns(vertexIns), m_explicitFragOuts(fragOuts) {}
        TMglGlslIoResolver(const glslang::TProgram& program, const EShLanguage stage, const ExplicitVarSlotMap& vertexIns, const ExplicitVarSlotMap& fragOuts):
                TMglGlslIoResolver(*program.getIntermediate(stage), vertexIns, fragOuts) {}
        void reserverStorageSlot(glslang::TVarEntryInfo& ent, TInfoSink& infoSink) override;
    protected:
        const ExplicitVarSlotMap& m_explicitVertexIns;
        const ExplicitVarSlotMap& m_explicitFragOuts;
    };
}


