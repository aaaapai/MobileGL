// MobileGL - MobileGL/MG_State/GLState/ErrorState/Error.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "ErrorCode.h"
#include "ErrorInfo.h"

namespace MobileGL {
    struct Error {
        ErrorCode code;
        SharedPtr<ErrorInfo> info;
    };

    namespace MG_State {
        namespace GLState {
            class ErrorState {
            public:
                void RecordError(ErrorCode code, SharedPtr<ErrorInfo> info = nullptr);
                Bool HasNonGLError() const;
                Optional<const Error> PeekNonGLError() const;
                Optional<Error> PopNonGLError();
                Bool HasGLError() const;
                Optional<const Error> PeekGLError() const;
                Optional<Error> PopGLError();
                void Clear();

            private:
                Vector<Error> m_errors;
                Vector<Error> m_nonGLErrors;
            };
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL