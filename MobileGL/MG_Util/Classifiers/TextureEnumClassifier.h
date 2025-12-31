// MobileGL - MobileGL/MG_Util/Classifiers/TextureEnumClassifier.h
// Copyright (c) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include <MG_State/GLState/TextureState/TextureObject.h>

namespace MobileGL {
    namespace MG_Util {
        bool IsDepthFormatInternalFormat(TextureInternalFormat internalformat);
        bool IsStencilFormatInternalFormat(TextureInternalFormat internalformat);
    } // namespace MG_Util
} // namespace MobileGL