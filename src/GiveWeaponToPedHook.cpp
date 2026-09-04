#include "GiveWeaponToPedHook.h"
#include "NativeSigHook.h"

#include <cstdint>
#include <cstring>

namespace
{
    constexpr const char* kPattern =
        "40 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? F3 0F 10 45 ? 33 C0";

    NativeSigHook<int64_t, int64_t, int32_t, int32_t, char, char, int32_t, char,
        int32_t, int32_t, int32_t, char, int32_t, char> g_hook{ "GIVE_WEAPON_TO_PED", kPattern };

    // NativeDB types p7/p8/permanentDegradation as float -- that reflects
    // what the script VM actually pushes, so trust it over Hex-Rays' `int`
    // typing for these three slots (same 4 bytes either way; only the
    // interpretation differs). memcpy avoids UB from reinterpret_cast punning.
    float AsFloat(int32_t bits)
    {
        float f;
        std::memcpy(&f, &bits, sizeof(f));
        return f;
    }

    int64_t Detour(int64_t a1, int32_t a2, int32_t a3, char a4, char a5, int32_t a6, char a7,
        int32_t a8, int32_t a9, int32_t a10, char a11, int32_t a12, char a13)
    {
        int64_t result = g_hook.CallOriginal(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);

        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "GIVE_WEAPON_TO_PED(ped=0x%llX, weaponHash=0x%08X, ammoCount=%d, "
                "forceInHand=%d, forceInHolster=%d, attachPoint=%d, allowMultiple=%d, "
                "p7=%f, p8=%f, addReason=0x%08X, ignoreUnlocks=%d, permDegradation=%f, p12=%d) -> 0x%llX",
                a1, (unsigned)a2, a3, (int)a4, (int)a5, a6, (int)a7,
                AsFloat(a8), AsFloat(a9), (unsigned)a10, (int)a11, AsFloat(a12), (int)a13, result);
        }

        return result;
    }
}

namespace GiveWeaponToPedHook
{
    bool Install() { return g_hook.Install(&Detour); }
    void Remove() { g_hook.Remove(); }
}
