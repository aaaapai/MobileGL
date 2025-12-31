// MobileGL - MobileGL/MG_Util/ShaderTranspiler/glslang/TVarEntryInfo.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
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
