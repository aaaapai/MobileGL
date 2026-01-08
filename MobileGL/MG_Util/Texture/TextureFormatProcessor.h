// MobileGL - MobileGL/MG_Util/Texture/TextureFormatProcessor.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL::MG_Util::TextureFormatProcessor {
    void NormalizePixelFormat(GLenum internalFormat, GLenum* outInternalFormat, GLenum* outFormat, GLenum* outType);
} // namespace MobileGL::MG_Util::TextureFormatProcessor