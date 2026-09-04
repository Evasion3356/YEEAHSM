#include "RemoveWeaponFromPedHook.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kPattern =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 8B F2 45 8B F1";

    NativeSigHook<void, int64_t, uint32_t, int64_t, uint32_t> g_hook{ "REMOVE_WEAPON_FROM_PED", kPattern };

    void Detour(int64_t a1, uint32_t a2, int64_t a3, uint32_t a4)
    {
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "REMOVE_WEAPON_FROM_PED(ped=0x%llX, weaponHash=0x%08X, p2=0x%llX, removeReason=0x%08X)",
                a1, a2, a3, a4);
        }
        g_hook.CallOriginal(a1, a2, a3, a4);
    }
}

namespace RemoveWeaponFromPedHook
{
    bool Install() { return g_hook.Install(&Detour); }
    void Remove() { g_hook.Remove(); }
}
