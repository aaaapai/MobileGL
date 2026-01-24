// MobileGL - MobileGL/MG_Util/ShaderTranspiler/FloatEqualsZeroEliminationPass.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include "source/opt/pass.h"

#include <Includes.h>

namespace MobileGL {
    namespace MG_Util {
        namespace ShaderTranspiler {
            class FloatEqualsZeroEliminationPass: public spvtools::opt::Pass {
            public:
                const char* name() const override { return "float-equals-zero-elimination"; }
                Status Process() override;
            private:
                const float K_EPSILON = 0.0001f;
            };
        } // namespace ShaderTranspiler
    } // namespace MG_Util
} // namespace MobileGL
