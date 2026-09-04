#include "DoesEntityExistHook.h"
#include "PatternScan.h"
#include "Logger.h"

#include <MinHook.h>
#include <cstdint>

namespace
{
    // IDA prologue for DOES_ENTITY_EXIST:
    //   push rbx; sub rsp, ??; cmp ecx, ??; jne ??; xor al, al
    constexpr const char* kPattern = "40 53 48 83 EC ? 83 F9 ? 75 ? 32 C0";

    using DoesEntityExistFn = char (*)(int32_t);

    void* g_target = nullptr;
    DoesEntityExistFn g_original = nullptr;
    bool g_hooked = false;

    char DetourDoesEntityExist(int32_t entity)
    {
        char result = g_original(entity);

        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "DOES_ENTITY_EXIST_NATIVE(entity=%d) -> %d",
                entity, (int)result);
        }

        return result;
    }
}

namespace DoesEntityExistHook
{
    bool Install()
    {
        g_target = PatternScan::Find(kPattern);
        if (!g_target)
        {
            Logger::Log("DoesEntityExistHook: pattern not found -- game version mismatch or bad AOB.");
            return false;
        }

        MH_STATUS status = MH_CreateHook(g_target,
            reinterpret_cast<void*>(&DetourDoesEntityExist),
            reinterpret_cast<void**>(&g_original));
        if (status != MH_OK)
        {
            Logger::LogFormatted("DoesEntityExistHook: MH_CreateHook failed, status=%d", (int)status);
            g_target = nullptr;
            return false;
        }

        status = MH_EnableHook(g_target);
        if (status != MH_OK)
        {
            Logger::LogFormatted("DoesEntityExistHook: MH_EnableHook failed, status=%d", (int)status);
            MH_RemoveHook(g_target);
            g_target = nullptr;
            return false;
        }

        g_hooked = true;
        Logger::LogFormatted("DoesEntityExistHook: hooked DOES_ENTITY_EXIST_NATIVE at 0x%p", g_target);
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
