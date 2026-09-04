#include "LongarmsStoreOnDismountHooks.h"
#include "NativeSigHook.h"

#include <cstdint>

namespace
{
    constexpr const char* kGetPattern =
        "40 53 48 83 EC ? 8A DA 48 85 C9 74 ? E8";
    constexpr const char* kSetPattern =
        "48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 41 8A D8";

    NativeSigHook<char, int64_t, char> g_getHook{ "GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT", kGetPattern };
    NativeSigHook<void, int64_t, char, char> g_setHook{ "SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT", kSetPattern };

    char DetourGet(int64_t a1, char a2)
    {
        char realResult = g_getHook.CallOriginal(a1, a2);

        Logger::LogFormatted(
            "GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT(pedPtr=0x%llX, bitSel=%d) real=%d -> forced 0",
            a1, (int)a2, (int)realResult);

        return 0;
    }

    void DetourSet(int64_t a1, char a2, char a3)
    {
        Logger::LogFormatted(
            "SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT(pedPtr=0x%llX, storeLongarms=%d, bitSel=%d) -- forcing storeLongarms=0",
            a1, (int)a2, (int)a3);

        g_setHook.CallOriginal(a1, 0, a3);
    }
}

namespace LongarmsStoreOnDismountHooks
{
    bool Install(bool quiet)
    {
        bool ok1 = g_getHook.Install(&DetourGet, quiet);
        bool ok2 = g_setHook.Install(&DetourSet, quiet);
        return ok1 && ok2;
    }

    void Remove()
    {
        g_getHook.Remove();
        g_setHook.Remove();
    }
}
