// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/TVarEntryInfo.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

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
        TIntermSymbol* symbol;
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
} // namespace glslang
