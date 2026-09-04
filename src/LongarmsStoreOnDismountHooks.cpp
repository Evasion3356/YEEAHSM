#include "LongarmsStoreOnDismountHooks.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kGetPattern =
        "40 53 48 83 EC ? 8A DA 48 85 C9 74 ? E8";
    constexpr const char* kSetPattern =
        "48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 41 8A D8";

    // Index of the E8 (CALL rel32) opcode byte within kGetPattern -- the
    // call to the ped->WeaponComponent resolver (IDA: sub_140CC5C00)
    // immediately follows the "test rcx,rcx / jz" that checks a1, and the
    // pattern was cut to end exactly there. 14 tokens, E8 is the 14th (index 13).
    constexpr size_t kCallOpcodeOffset = 13;

    NativeSigHook<char, int64_t, char> g_getHook{ "GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT", kGetPattern };
    NativeSigHook<void, int64_t, char, char> g_setHook{ "SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT", kSetPattern };

    using GetWeaponComponentFn = int64_t (*)(int64_t);
    GetWeaponComponentFn g_getWeaponComponent = nullptr;

    // Decodes the CALL rel32 sitting at kGetPattern's tail to recover
    // sub_140CC5C00's real address, instead of needing its own signature.
    bool ResolveWeaponComponentHelper()
    {
        auto getAddr = reinterpret_cast<uint8_t*>(g_getHook.GetTarget());
        if (!getAddr)
            return false;

        uint8_t* callOpcode = getAddr + kCallOpcodeOffset;
        if (*callOpcode != 0xE8)
        {
            Logger::LogFormatted(
                "LongarmsStoreOnDismountHooks: expected E8 at GET+%zu, found 0x%02X -- pattern shifted, can't resolve WeaponComponent helper.",
                kCallOpcodeOffset, (unsigned)*callOpcode);
            return false;
        }

        int32_t disp = *reinterpret_cast<int32_t*>(callOpcode + 1);
        uint8_t* nextInstr = callOpcode + 5; // 1 opcode byte + 4 displacement bytes
        g_getWeaponComponent = reinterpret_cast<GetWeaponComponentFn>(nextInstr + disp);

        Logger::LogFormatted("LongarmsStoreOnDismountHooks: resolved WeaponComponent helper at 0x%p", (void*)g_getWeaponComponent);
        return true;
    }

    void LogBitAddress(const char* nativeName, int64_t pedPtr)
    {
        if (!g_getWeaponComponent)
        {
            Logger::LogFormatted("%s(pedPtr=0x%llX): WeaponComponent helper not resolved, can't compute bit address.", nativeName, pedPtr);
            return;
        }

        int64_t component = g_getWeaponComponent(pedPtr);
        if (!component)
        {
            Logger::LogFormatted("%s(pedPtr=0x%llX): WeaponComponent resolved to null.", nativeName, pedPtr);
            return;
        }

        void* bitAddr = reinterpret_cast<void*>(component + 0x1B5);
        Logger::LogFormatted("%s: pedPtr=0x%llX  weaponComponent=0x%llX  bitAddr=0x%p  <- put your Read BP here",
            nativeName, pedPtr, component, bitAddr);
    }

    char DetourGet(int64_t a1, char a2)
    {
        char result = g_getHook.CallOriginal(a1, a2);

        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT(pedPtr=0x%llX, bitSel=%d) -> %d",
                a1, (int)a2, (int)result);
            LogBitAddress("GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT", a1);
        }

        return result;
    }

    void DetourSet(int64_t a1, char a2, char a3)
    {
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT(pedPtr=0x%llX, storeLongarms=%d, bitSel=%d)",
                a1, (int)a2, (int)a3);
            LogBitAddress("SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT", a1);
        }

        g_setHook.CallOriginal(a1, a2, a3);
    }
}

namespace LongarmsStoreOnDismountHooks
{
    bool Install()
    {
        bool ok1 = g_getHook.Install(&DetourGet);
        bool ok2 = g_setHook.Install(&DetourSet);

        if (ok1)
            ResolveWeaponComponentHelper();

        return ok1 && ok2;
    }

    void Remove()
    {
        g_getHook.Remove();
        g_setHook.Remove();
        g_getWeaponComponent = nullptr;
    }
}
