#pragma once

namespace MobileGL {
    enum class ErrorCode {
        NoError = 0,
        InvalidEnum,
        InvalidValue,
        InvalidOperation,
        InvalidFramebufferOperation,
        OutOfMemory,
        StackUnderflow,
        StackOverflow
    };
}
