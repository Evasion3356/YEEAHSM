#include "Logger.h"
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
    bool g_enabled = false;

    std::string GetLogPath()
    {
        HMODULE hSelf = nullptr;
        GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&GetLogPath), &hSelf);

        char path[MAX_PATH]{};
        GetModuleFileNameA(hSelf, path, MAX_PATH);

        std::string p(path);
        auto dot = p.find_last_of('.');
        if (dot != std::string::npos)
            p = p.substr(0, dot);
        return p + ".log";
    }

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
        g_file.open(GetLogPath(), std::ios::out | std::ios::app);
        g_file << "\n----- YEEAHS session start -----\n";
        g_file.flush();
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_file.is_open())
        {
            g_file << "----- YEEAHS session end -----\n";
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

    bool ToggleEnabled()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_enabled = !g_enabled;
        return g_enabled;
    }

    bool IsEnabled()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        return g_enabled;
    }
}
