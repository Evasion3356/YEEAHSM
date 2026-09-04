#pragma once
#include <string>

// Thread-safe file logger. Off by default -- press F9 in-game to toggle so
// you can bracket exactly the mount/dismount repro window instead of drowning
// in every native call from process start.
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

    bool ToggleEnabled();
    bool IsEnabled();
}

#else

namespace Logger
{
    inline void Init() {}
    inline void Shutdown() {}

    inline void Log(const std::string&) {}
    inline void LogFormatted(const char*, ...) {}

    inline bool ToggleEnabled() { return false; }
    inline bool IsEnabled() { return false; }
}

#endif
