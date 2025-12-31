// MobileGL - MobileGL/MG_State/GLState/ErrorState/ErrorInfo.h
// Copyright (C) 2025 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v2.1:
// http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
// SPDX-License-Identifier: LGPL-2.1-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    class ErrorInfo {
    public:
        virtual ~ErrorInfo() = default;
        virtual String ToString() const = 0;
    };

    class GenericErrorInfo : public ErrorInfo {
    public:
        explicit GenericErrorInfo(String message) : m_message(Move(message)) {}
        explicit GenericErrorInfo(String m_prefix, String message)
            : m_message(Move(message)), m_prefix(Move(m_prefix)) {}
        explicit GenericErrorInfo(String m_prefix, String m_prefix_2, String message)
            : m_message(Move(message)), m_prefix(Move(m_prefix)), m_prefix_2(Move(m_prefix_2)) {}

        String ToString() const override {
            StringStream ss;
            if (m_prefix.has_value()) {
                ss << "[" << m_prefix.value() << "] ";
            }
            if (m_prefix_2.has_value()) {
                ss << "[" << m_prefix_2.value() << "] ";
            }
            ss << m_message;
            return ss.str();
        }

    private:
        String m_message;
        Optional<String> m_prefix;
        Optional<String> m_prefix_2;
    };
} // namespace MobileGL
