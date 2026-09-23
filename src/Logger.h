#pragma once
#include <string>

// Thread-safe file logger. Writes YEEAHSM.log next to the .asi, or to
// %LOCALAPPDATA%\RDR2ASIMods\YEEAHSM.log when the game folder isn't
// writable (see LogFallback.h).
//
// Debug-only: Logger.cpp is excluded from the Release build, and every
// Logger:: call below compiles to a no-op there, so call sites never need
// #ifdefs.
#ifdef _DEBUG

namespace Logger
{
    void Init();
    void Shutdown();

    void Log(const std::string& line);
    void LogFormatted(const char* fmt, ...);
}

#else

namespace Logger
{
    inline void Init() {}
    inline void Shutdown() {}

    inline void Log(const std::string&) {}
    inline void LogFormatted(const char*, ...) {}
}

#endif
