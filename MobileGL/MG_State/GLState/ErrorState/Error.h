#pragma once

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
                bool HasNonGLError() const;
                Optional<const Error> PeekNonGLError() const;
                Optional<Error> PopNonGLError();
                bool HasGLError() const;
                Optional<const Error> PeekGLError() const;
                Optional<Error> PopGLError();
                void Clear();

            private:
                Vector<Error> m_errors;
                Vector<Error> m_nonGLErrors;
            };
        }
    }
}