#include "YEEAHSMLog.h"

#include <atomic>
#include <cstdarg>
#include <cstdio>

namespace YEEAHSM::Log
{
    namespace
    {
        std::atomic<Sink> g_sink{ nullptr };
    }

    void SetSink(Sink sink)
    {
        g_sink.store(sink);
    }

    void Write(const char* line)
    {
        if (const Sink sink = g_sink.load())
            sink(line);
    }

    void Formatted(const char* fmt, ...)
    {
        const Sink sink = g_sink.load();
        if (!sink)
            return;
        char line[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(line, sizeof(line), fmt, args);
        va_end(args);
        sink(line);
    }
}
