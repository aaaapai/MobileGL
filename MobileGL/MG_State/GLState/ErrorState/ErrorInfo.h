// MobileGL - MobileGL/MG_State/GLState/ErrorState/ErrorInfo.h
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#pragma once
#include <Includes.h>

namespace MobileGL {
    class ErrorInfo {
    public:
        virtual ~ErrorInfo() = default;
        virtual String toString() const = 0;
    };

    class GenericErrorInfo : public ErrorInfo {
    public:
        explicit GenericErrorInfo(String message) : m_message(Move(message)) {}
        explicit GenericErrorInfo(String m_prefix, String message)
            : m_message(Move(message)), m_prefix(Move(m_prefix)) {}
        explicit GenericErrorInfo(String m_prefix, String m_prefix_2, String message)
            : m_message(Move(message)), m_prefix(Move(m_prefix)), m_prefix_2(Move(m_prefix_2)) {}

        String toString() const override {
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
