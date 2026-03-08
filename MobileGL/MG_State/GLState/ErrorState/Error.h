// MobileGL - MobileGL/MG_State/GLState/ErrorState/Error.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>
#include "ErrorCode.h"
#include "ErrorInfo.h"

namespace MobileGL {
    struct Error {
        ErrorCode code;
        UniquePtr<ErrorInfo> info;
    };

    namespace MG_State::GLState {
        class ErrorState {
        public:
            void RecordError(ErrorCode code, UniquePtr<ErrorInfo> info);
            Bool HasNonGLError() const;
            Optional<const Error*> PeekNonGLError() const;
            Optional<UniquePtr<Error>> PopNonGLError();
            Bool HasGLError() const;
            Optional<const Error*> PeekGLError() const;
            Optional<UniquePtr<Error>> PopGLError();
            void Clear();

        private:
            Vector<UniquePtr<Error>> m_errors;
            Vector<UniquePtr<Error>> m_nonGLErrors;
        };
    } // namespace MG_State::GLState
} // namespace MobileGL
