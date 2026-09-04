#include "SdkHook.h"
#include "Logger.h"

#include <windows.h>
#include <MinHook.h>

namespace
{
    using nativeInit_t = void (*)(UINT64);
    using nativePush64_t = void (*)(UINT64);
    using nativeCall_t = PUINT64 (*)();

    nativeInit_t   g_origNativeInit = nullptr;
    nativePush64_t g_origNativePush64 = nullptr;
    nativeCall_t   g_origNativeCall = nullptr;

    void* g_nativeInitAddr = nullptr;
    void* g_nativePush64Addr = nullptr;
    void* g_nativeCallAddr = nullptr;

    bool g_nativeInitHooked = false;
    bool g_nativePush64Hooked = false;
    bool g_nativeCallHooked = false;

    void DetourNativeInit(UINT64 hash)
    {
        if (Logger::IsEnabled())
            Logger::LogFormatted("nativeInit(hash=0x%016llX)", hash);
        g_origNativeInit(hash);
    }

    void DetourNativePush64(UINT64 val)
    {
        if (Logger::IsEnabled())
            Logger::LogFormatted("  nativePush64(val=0x%016llX)", val);
        g_origNativePush64(val);
    }

    PUINT64 DetourNativeCall()
    {
        PUINT64 result = g_origNativeCall();
        if (Logger::IsEnabled())
        {
            Logger::LogFormatted("  nativeCall() -> ptr=0x%p first8=0x%016llX",
                (void*)result, result ? *result : 0ULL);
        }
        return result;
    }

    bool HookOne(const char* exportName, void* target, void* detour, void** originalOut)
    {
        MH_STATUS status = MH_CreateHook(target, detour, originalOut);
        if (status != MH_OK)
        {
            Logger::LogFormatted("MH_CreateHook failed for %s: status=%d", exportName, (int)status);
            return false;
        }

        status = MH_EnableHook(target);
        if (status != MH_OK)
        {
            Logger::LogFormatted("MH_EnableHook failed for %s: status=%d", exportName, (int)status);
            MH_RemoveHook(target);
            return false;
        }

        Logger::LogFormatted("hooked (known-good export): %-14s addr=0x%p", exportName, target);
        return true;
    }
}

namespace SdkHook
{
    void InstallAll()
    {
        HMODULE hShv = GetModuleHandleA("ScriptHookRDR2.dll");
        if (!hShv)
        {
            Logger::Log("SdkHook: ScriptHookRDR2.dll not found in process -- cannot resolve exports.");
            return;
        }

        g_nativeInitAddr = reinterpret_cast<void*>(GetProcAddress(hShv, "nativeInit"));
        g_nativePush64Addr = reinterpret_cast<void*>(GetProcAddress(hShv, "nativePush64"));
        g_nativeCallAddr = reinterpret_cast<void*>(GetProcAddress(hShv, "nativeCall"));

        if (g_nativeInitAddr)
        {
            g_nativeInitHooked = HookOne("nativeInit", g_nativeInitAddr,
                reinterpret_cast<void*>(&DetourNativeInit),
                reinterpret_cast<void**>(&g_origNativeInit));
        }
        else
        {
            Logger::Log("SdkHook: GetProcAddress(nativeInit) failed.");
        }

        if (g_nativePush64Addr)
        {
            g_nativePush64Hooked = HookOne("nativePush64", g_nativePush64Addr,
                reinterpret_cast<void*>(&DetourNativePush64),
                reinterpret_cast<void**>(&g_origNativePush64));
        }
        else
        {
            Logger::Log("SdkHook: GetProcAddress(nativePush64) failed.");
        }

        if (g_nativeCallAddr)
        {
            g_nativeCallHooked = HookOne("nativeCall", g_nativeCallAddr,
                reinterpret_cast<void*>(&DetourNativeCall),
                reinterpret_cast<void**>(&g_origNativeCall));
        }
        else
        {
            Logger::Log("SdkHook: GetProcAddress(nativeCall) failed.");
        }
    }

    void RemoveAll()
    {
        if (g_nativeInitHooked)
        {
            MH_DisableHook(g_nativeInitAddr);
            MH_RemoveHook(g_nativeInitAddr);
            g_nativeInitHooked = false;
        }
        if (g_nativePush64Hooked)
        {
            MH_DisableHook(g_nativePush64Addr);
            MH_RemoveHook(g_nativePush64Addr);
            g_nativePush64Hooked = false;
        }
        if (g_nativeCallHooked)
        {
            MH_DisableHook(g_nativeCallAddr);
            MH_RemoveHook(g_nativeCallAddr);
            g_nativeCallHooked = false;
        }
    }
}
