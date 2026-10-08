#pragma once

// Logging for the YEEAHSM library (YEEAHSMLib.vcxproj). The library has no
// log file of its own: whoever links it (this repo's ASI, or a trainer that
// pulls this repo in as a submodule) sets a sink and decides where lines
// go. Lines written before a sink is set are dropped. The ASI points the
// sink at Logger, so the library's lines stay Debug-only there.
namespace YEEAHSM::Log
{
    using Sink = void (*)(const char* line);

    void SetSink(Sink sink);
    void Write(const char* line);
    // printf-style, formatted into a line and sent to the sink.
    void Formatted(const char* fmt, ...);
}
