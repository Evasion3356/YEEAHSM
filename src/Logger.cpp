#include "Logger.h"
#include "LogFallback.h"
#include <windows.h>
#include <fstream>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace
{
    std::ofstream g_file;
    std::mutex g_mutex;
    LogFallback::Resolved g_target;

    std::string Timestamp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm tmBuf{};
        localtime_s(&tmBuf, &t);

        std::ostringstream oss;
        oss << std::put_time(&tmBuf, "%H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }
}

namespace Logger
{
    void Init()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_target = LogFallback::Resolve(LogFallback::ModuleDirectory(), L"YEEAHSM.log", LogFallback::FallbackDirectory());
        if (g_target.path.empty())
            return;
        g_file.open(g_target.path, std::ios::out | std::ios::app);
        g_file << "\n----- YEEAHSM session start -----\n";
        if (g_target.usedFallback)
            g_file << "Log redirected here: could not write " << LogFallback::ToUtf8(g_target.rejectedPath) << "\n";
        g_file.flush();
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_file.is_open())
        {
            g_file << "----- YEEAHSM session end -----\n";
            g_file.close();
        }
    }

    void Log(const std::string& line)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_file.is_open())
            return;
        g_file << '[' << Timestamp() << "] " << line << '\n';
        g_file.flush();
    }

    void LogFormatted(const char* fmt, ...)
    {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        Log(buf);
    }
}
