#include "SetCurrentPedWeaponHook.h"
#include "NativeSigHook.h"
#include "CampScriptCheck.h"

#include <cstdint>
#include <intrin.h>

namespace
{
    constexpr const char* kPattern =
        "48 85 C9 0F 84 ? ? ? ? 48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 45 8B F1 41 8A E8 8B FA";

    NativeSigHook<void, int64_t, int64_t, int64_t, int64_t, char, char> g_hook{ "SET_CURRENT_PED_WEAPON", kPattern };

    void Detour(int64_t a1, int64_t a2, int64_t a3, int64_t a4, char a5, char a6)
    {
        bool discard = CampScriptCheck::ShouldDiscard("SET_CURRENT_PED_WEAPON");

        Logger::LogFormatted(
            "SET_CURRENT_PED_WEAPON(ped=0x%llX, weaponHash=0x%llX, equipNow=%lld, attachPoint=%lld, p4=%d, p5=%d)  retaddr=0x%p  %s",
            a1, a2, a3, a4, (int)a5, (int)a6, _ReturnAddress(), discard ? "[DISCARDED]" : "[passed through]");

        if (!discard)
            g_hook.CallOriginal(a1, a2, a3, a4, a5, a6);
    }
}

namespace SetCurrentPedWeaponHook
{
    bool Install(bool quiet) { return g_hook.Install(&Detour, quiet); }
    void Remove() { g_hook.Remove(); }
}
