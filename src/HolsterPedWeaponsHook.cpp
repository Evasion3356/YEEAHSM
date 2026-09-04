#include "HolsterPedWeaponsHook.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kPattern =
        "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 45 8A F1 41 8A F8";

    NativeSigHook<void, int64_t, char, char, char, char> g_hook{ "HOLSTER_PED_WEAPONS", kPattern };

    void Detour(int64_t a1, char a2, char a3, char a4, char a5)
    {
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "HOLSTER_PED_WEAPONS(ped=0x%llX, p1=%d, p2=%d, p3=%d, immediately=%d)",
                a1, (int)a2, (int)a3, (int)a4, (int)a5);
        }
        g_hook.CallOriginal(a1, a2, a3, a4, a5);
    }
}

namespace HolsterPedWeaponsHook
{
    bool Install() { return g_hook.Install(&Detour); }
    void Remove() { g_hook.Remove(); }
}
