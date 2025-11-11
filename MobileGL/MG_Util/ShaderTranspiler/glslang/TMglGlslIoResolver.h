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
        TMglGlslIoResolver(const glslang::TIntermediate& intermediate, const ExplicitVarSlotMap& attribLocations):
            TDefaultGlslIoResolver(intermediate), m_explicitAttribLocations(attribLocations) {}
        TMglGlslIoResolver(const glslang::TProgram& program, const EShLanguage stage, const ExplicitVarSlotMap& attribLocations):
            TDefaultGlslIoResolver(*program.getIntermediate(stage)),
            m_explicitAttribLocations(attribLocations) {}
        void reserverStorageSlot(glslang::TVarEntryInfo& ent, TInfoSink& infoSink) override;
    protected:
        const ExplicitVarSlotMap& m_explicitAttribLocations;
    };
}


