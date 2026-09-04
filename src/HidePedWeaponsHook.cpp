#include "HidePedWeaponsHook.h"
#include "NativeSigHook.h"
#include "CampScriptCheck.h"

#include <cstdint>
#include <intrin.h>

namespace
{
    constexpr const char* kPattern =
        "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 70 ? 55 48 8B EC 48 83 EC ? 45 8A F0 8B FA";

    NativeSigHook<void, int64_t, int32_t, char> g_hook{ "HIDE_PED_WEAPONS", kPattern };

    void Detour(int64_t a1, int32_t a2, char a3)
    {
        bool discard = CampScriptCheck::ShouldDiscard("HIDE_PED_WEAPONS");

        Logger::LogFormatted(
            "HIDE_PED_WEAPONS(ped=0x%llX, p0=%d, immediately=%d)  retaddr=0x%p  %s",
            a1, a2, (int)a3, _ReturnAddress(), discard ? "[DISCARDED]" : "[passed through]");

        if (!discard)
            g_hook.CallOriginal(a1, a2, a3);
    }
}

namespace HidePedWeaponsHook
{
    bool Install(bool quiet) { return g_hook.Install(&Detour, quiet); }
    void Remove() { g_hook.Remove(); }
}
