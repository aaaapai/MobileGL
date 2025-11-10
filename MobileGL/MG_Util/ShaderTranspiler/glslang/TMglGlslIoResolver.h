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

class TMglGlslIoResolver: public glslang::TDefaultGlslIoResolver {
    int resolveInOutLocation(EShLanguage stage, glslang::TVarEntryInfo& ent) override;
    int resolveUniformLocation(EShLanguage /*stage*/, glslang::TVarEntryInfo& ent) override;
};


