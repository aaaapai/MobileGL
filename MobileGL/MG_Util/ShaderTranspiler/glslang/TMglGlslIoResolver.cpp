//
// Created by Swung 0x48 on 2025/11/10.
//

#include "TMglGlslIoResolver.h"

int TMglGlslIoResolver::resolveInOutLocation(EShLanguage stage, glslang::TVarEntryInfo &ent) {
    return glslang::TDefaultGlslIoResolver::resolveInOutLocation(stage, ent);
}

int TMglGlslIoResolver::resolveUniformLocation(EShLanguage stage, glslang::TVarEntryInfo &ent) {
    return glslang::TDefaultGlslIoResolver::resolveUniformLocation(stage, ent);
}
