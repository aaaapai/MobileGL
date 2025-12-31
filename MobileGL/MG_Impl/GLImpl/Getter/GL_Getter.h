// MobileGL - MobileGL/MG_Impl/GLImpl/Getter/GL_Getter.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    namespace MG_Impl::GLImpl {
        /* @INSERTION_POINT:FUNCTION_DECLARATION@ */
        const GLubyte* GetString(GLenum name);
        const GLubyte* GetStringi(GLenum name, GLuint index);
        void GetIntegerv(GLenum pname, GLint* params);
        GLenum GetError();
    } // namespace MG_Impl::GLImpl
} // namespace MobileGL