#include "StowWeaponsHook.h"
#include "NativeSigHook.h"
#include "PatternScan.h"

#include <cstdint>
#include <atomic>
#include <intrin.h>

namespace
{
    constexpr const char* kPattern =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 45 33 F6 45 8B E0";

    NativeSigHook<void, int64_t, int64_t, uint32_t, int64_t, char> g_hook{ "StowWeapons (sub_14089EE14)", kPattern };

    std::atomic<uint64_t> g_callCount{ 0 };

    void Detour([[maybe_unused]] int64_t a1, [[maybe_unused]] int64_t a2, [[maybe_unused]] uint32_t a3,
        [[maybe_unused]] int64_t a4, [[maybe_unused]] char a5)
    {
#ifdef _DEBUG
        // Debug-only: in Release the log call is a no-op, so don't pay for
        // the call counter or the PE-header walk in ToModuleOffset().
        uint64_t count = g_callCount.fetch_add(1, std::memory_order_relaxed) + 1;

        size_t retOffset = PatternScan::ToModuleOffset(_ReturnAddress());

        Logger::LogFormatted(
            "StowWeapons call #%llu: a1=0x%llX a2=0x%llX a3=%u a4=0x%llX a5=%d  retaddr=RDR2.exe+0x%zX  [DISCARDED -- not calling original]",
            count, a1, a2, a3, a4, (int)a5, retOffset);
#endif

        // Deliberately not calling g_hook.CallOriginal() -- this is the same
        // effect as the manual RETN patch that proved this function is what
        // moves the weapon onto the horse, just applied via hook instead of
        // a static byte patch. Logging still tells us every call site
        // (count + args + real retaddr) that would have fired.
    }
}

namespace StowWeaponsHook
{
    bool Install(bool quiet) { return g_hook.Install(&Detour, quiet); }
    void Remove() { g_hook.Remove(); }
}
