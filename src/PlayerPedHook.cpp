#include "PlayerPedHook.h"
#include "PatternScan.h"
#include "Logger.h"

#include <MinHook.h>
#include <cstdint>

namespace
{
    // IDA prologue for GET_PLAYER_PED_NATIVE:
    //   push rbx; sub rsp, ??; xor ebx, ebx; cmp ecx, 0FFh  (== 255)
    constexpr const char* kPattern = "40 53 48 83 EC ? 33 DB 81 F9";

    using GetPlayerPedFn = int64_t (*)(int64_t, int64_t);

    void* g_target = nullptr;
    GetPlayerPedFn g_original = nullptr;
    bool g_hooked = false;

    int64_t DetourGetPlayerPed(int64_t a1, int64_t a2)
    {
        int64_t result = g_original(a1, a2);

        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "GET_PLAYER_PED_NATIVE(a1=0x%llX, a2=0x%llX) -> 0x%llX",
                a1, a2, result);
        }

        return result;
    }
}

namespace PlayerPedHook
{
    bool Install()
    {
        g_target = PatternScan::Find(kPattern);
        if (!g_target)
        {
            Logger::Log("PlayerPedHook: pattern not found -- game version mismatch or bad AOB.");
            return false;
        }

        MH_STATUS status = MH_CreateHook(g_target,
            reinterpret_cast<void*>(&DetourGetPlayerPed),
            reinterpret_cast<void**>(&g_original));
        if (status != MH_OK)
        {
            Logger::LogFormatted("PlayerPedHook: MH_CreateHook failed, status=%d", (int)status);
            g_target = nullptr;
            return false;
        }

        status = MH_EnableHook(g_target);
        if (status != MH_OK)
        {
            Logger::LogFormatted("PlayerPedHook: MH_EnableHook failed, status=%d", (int)status);
            MH_RemoveHook(g_target);
            g_target = nullptr;
            return false;
        }

        g_hooked = true;
        Logger::LogFormatted("PlayerPedHook: hooked GET_PLAYER_PED_NATIVE at 0x%p", g_target);
        return true;
    }

    void Remove()
    {
        if (!g_hooked)
            return;

        MH_DisableHook(g_target);
        MH_RemoveHook(g_target);
        g_hooked = false;
        g_target = nullptr;
    }
}
