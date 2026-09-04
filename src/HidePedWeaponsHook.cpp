#include "HidePedWeaponsHook.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kPattern =
        "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 70 ? 55 48 8B EC 48 83 EC ? 45 8A F0 8B FA";

    NativeSigHook<void, int64_t, int32_t, char> g_hook{ "HIDE_PED_WEAPONS", kPattern };

    void Detour(int64_t a1, int32_t a2, char a3)
    {
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "HIDE_PED_WEAPONS(ped=0x%llX, p0=%d, immediately=%d)",
                a1, a2, (int)a3);
        }
        g_hook.CallOriginal(a1, a2, a3);
    }
}

namespace HidePedWeaponsHook
{
    bool Install() { return g_hook.Install(&Detour); }
    void Remove() { g_hook.Remove(); }
}
