#include "../../Includes.h"

namespace MobileGL {
    namespace MG_Util::Debug {
        static FILE* s_logFile = nullptr;
        static std::mutex s_mutex;

        constexpr char* GetOSName() {
#if defined(_WIN32)
            return (char*)"Windows";
#elif defined(__ANDROID__)
            return (char*)"Android";
#elif defined(__APPLE__)
            return (char*)"macOS";
#elif defined(__linux__)
            return (char*)"Linux";
#else
            return (char*)"UnknownOS";
#endif
        }

        std::string GetThreadName() {
            char buffer[64] = { 0 };
#if defined(_WIN32) && !defined(__MINGW32__)
            PWSTR desc = nullptr;
            if (SUCCEEDED(GetThreadDescription(GetCurrentThread(), &desc))) {
                WideCharToMultiByte(CP_UTF8, 0, desc, -1, buffer, sizeof(buffer), nullptr, nullptr);
                LocalFree(desc);
            }
#elif defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__) || defined(__MINGW32__)
            pthread_getname_np(pthread_self(), buffer, sizeof(buffer));
#endif
            return buffer[0] ? buffer : "UnknownThread";
        }

        std::string GetCurrentTime() {
            using namespace std::chrono;
            auto now = system_clock::now();
            std::time_t tt = system_clock::to_time_t(now);
            struct tm tm {};
#if defined(_WIN32)
            localtime_s(&tm, &tt);
#else
            localtime_r(&tt, &tm);
#endif
            char buf[16];
            std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
            return buf;
        }

        void InitFile() {
#if MOBILEGL_LOG_ENABLE_FILE
            if (!s_logFile && MOBILEGL_LOG_FILE_PATH && *MOBILEGL_LOG_FILE_PATH) {
                s_logFile = std::fopen(MOBILEGL_LOG_FILE_PATH, "w");
            }
#endif
        }

        void WriteToFile(const char* msg) {
#if MOBILEGL_LOG_ENABLE_FILE
            if (!s_logFile) InitFile();
            if (s_logFile) {
                std::fputs(msg, s_logFile);
                std::fflush(s_logFile);
            }
#endif
        }

        void Log(const char* levelTag, android_LogPriority androidLogLevel, const char* fmt, ...) {
            std::lock_guard<std::mutex> lock(s_mutex);

            std::string header = "[" + GetCurrentTime() + "] [" + GetOSName() + " " + GetThreadName() + "/" + levelTag + "]: ";

            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            int n = std::vsnprintf(buffer, sizeof(buffer), fmt, args);
            std::string out = header + std::string(buffer, n) + "\n";

#if MOBILEGL_LOG_ENABLE_CONSOLE
            std::fwrite(out.c_str(), 1, out.size(), stdout);
#endif

#if MOBILEGL_LOG_ENABLE_ANDROID && defined(__ANDROID__)
            __android_log_print(androidLogLevel, "MobileGL", "%s", out.c_str());
#endif

            WriteToFile(out.c_str());

            va_end(args);
        }

        void Close() {
#if MOBILEGL_LOG_ENABLE_FILE
            if (s_logFile) {
                std::fclose(s_logFile);
                s_logFile = nullptr;
            }
#endif
        }
    }
}