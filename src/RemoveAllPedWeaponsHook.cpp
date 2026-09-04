#include "RemoveAllPedWeaponsHook.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kPattern =
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B6 C2";

    NativeSigHook<void, int64_t, uint8_t, char> g_hook{ "REMOVE_ALL_PED_WEAPONS", kPattern };

    void Detour(int64_t a1, uint8_t a2, char a3)
    {
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "REMOVE_ALL_PED_WEAPONS(ped=0x%llX, p1=%u, p2=%d)",
                a1, (unsigned)a2, (int)a3);
        }
        g_hook.CallOriginal(a1, a2, a3);
    }
}

namespace RemoveAllPedWeaponsHook
{
    bool Install() { return g_hook.Install(&Detour); }
    void Remove() { g_hook.Remove(); }
}
