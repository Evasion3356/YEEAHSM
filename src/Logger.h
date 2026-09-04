#pragma once
#include <string>

// Thread-safe file logger. Off by default -- press F9 in-game to toggle so
// you can bracket exactly the mount/dismount repro window instead of drowning
// in every native call from process start.
namespace Logger
{
    void Init();
    void Shutdown();

    void Log(const std::string& line);
    void LogFormatted(const char* fmt, ...);

    bool ToggleEnabled();
    bool IsEnabled();
}
