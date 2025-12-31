// MobileGL - MobileGL/MG_State/GLState/ErrorState/Error.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#include "Error.h"
#include <MG_Util/Converters/GLToStr/GLEnumConverter.h>
#include <MG_Util/Converters/MGToGL/ErrorCodeConverter.h>

namespace MobileGL {
    namespace MG_State {
        namespace GLState {
            void ErrorState::RecordError(ErrorCode code, SharedPtr<ErrorInfo> info) {
                if (code == ErrorCode::NoError) {
                    MGLOG_E("Recording Non-OpenGL error:\n%s", info->ToString().c_str());
                    m_nonGLErrors.push_back(Error{code, info});
                } else {
                    MGLOG_E("Recording OpenGL error (%s):\n%s",
                            MG_Util::ConvertGLEnumToString(MG_Util::ConvertErrorCodeToGLEnum(code)).c_str(),
                            info->ToString().c_str());
                    m_errors.push_back(Error{code, info});
                }
            }

            Bool ErrorState::HasNonGLError() const {
                return !m_nonGLErrors.empty();
            }

            Optional<const Error> ErrorState::PeekNonGLError() const {
                if (m_nonGLErrors.empty()) return Optional<const Error>{};
                return Optional<const Error>{m_nonGLErrors.front()};
            }

            Optional<Error> ErrorState::PopNonGLError() {
                if (m_nonGLErrors.empty()) return Optional<const Error>{};
                auto error = Move(m_nonGLErrors.front());
                m_nonGLErrors.erase(m_nonGLErrors.begin());
                return Optional<const Error>{error};
            }

            Bool ErrorState::HasGLError() const {
                return !m_errors.empty();
            }

            Optional<const Error> ErrorState::PeekGLError() const {
                if (m_errors.empty()) return Optional<const Error>{};
                return Optional<const Error>{m_errors.front()};
            }

            Optional<Error> ErrorState::PopGLError() {
                if (m_errors.empty()) return Optional<const Error>{};
                auto error = Move(m_errors.front());
                m_errors.erase(m_errors.begin());
                return Optional<const Error>{error};
            }

            void ErrorState::Clear() {
                m_errors.clear();
                m_nonGLErrors.clear();
            }
        } // namespace GLState
    } // namespace MG_State
} // namespace MobileGL