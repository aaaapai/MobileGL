//
// Created by Swung 0x48 on 2025/11/10.
//

#pragma once
#include <glslang/Include/Types.h>

/*
 * This file is ripped from glslang/MachineIndependent/iomapper.cpp
 * As an temporary solution to define a custom glslang::TIoMapResolver
 * with previously public APIs
 */

namespace glslang {
    struct TVarEntryInfo {
        long long id;
        TIntermSymbol *symbol;
        bool live;
        TLayoutPacking upgradedToPushConstantPacking; // ElpNone means it hasn't been upgraded
        int newBinding;
        int newSet;
        int newLocation;
        int newComponent;
        int newIndex;
        EShLanguage stage;

        void clearNewAssignments();
    };
}
