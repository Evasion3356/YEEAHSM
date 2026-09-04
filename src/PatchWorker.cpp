#include "PatchWorker.h"
#include "DismountWeaponStripPatch.h"
#include "LongarmsStoreOnDismountHooks.h"
#include "Logger.h"

#include <windows.h>
#include <MinHook.h>
#include <atomic>
#include <mutex>

namespace
{
    constexpr DWORD kRetryIntervalMs = 500;
    constexpr DWORD kTimeoutMs = 120000; // 2 minutes
    constexpr DWORD kStatusEveryMs = 10000; // reassurance log every ~10s

    std::atomic<bool> g_stop{ false };
    std::mutex g_mutex; // guards the Install()/Remove() race between the
                         // worker thread and StopAndRevert()
    HANDLE g_thread = nullptr;

    bool g_patchApplied = false;
    bool g_longarmsHooksInstalled = false;
    bool g_minHookInitialized = false;

    DWORD WINAPI WorkerProc(LPVOID)
    {
        if (MH_Initialize() == MH_OK)
        {
            g_minHookInitialized = true;
        }
        else
        {
            Logger::Log("PatchWorker: MH_Initialize failed -- LongarmsStoreOnDismountHooks won't be available (byte patch still will).");
        }

        DWORD elapsed = 0;
        DWORD sinceStatus = 0;

        while (!g_stop.load(std::memory_order_relaxed) && elapsed < kTimeoutMs)
        {
            bool allDone;
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                if (g_stop.load(std::memory_order_relaxed))
                    break;

                if (!g_patchApplied)
                    g_patchApplied = DismountWeaponStripPatch::Install(/*quiet=*/true);

                if (!g_longarmsHooksInstalled && g_minHookInitialized)
                    g_longarmsHooksInstalled = LongarmsStoreOnDismountHooks::Install(/*quiet=*/true);

                allDone = g_patchApplied
                    && (g_longarmsHooksInstalled || !g_minHookInitialized);
            }

            if (allDone)
                return 0;

            Sleep(kRetryIntervalMs);
            elapsed += kRetryIntervalMs;
            sinceStatus += kRetryIntervalMs;

            if (sinceStatus >= kStatusEveryMs)
            {
                Logger::LogFormatted("PatchWorker: still waiting for game to finish unpacking (%lus elapsed)...", elapsed / 1000);
                sinceStatus = 0;
            }
        }

        if (!g_stop.load(std::memory_order_relaxed))
            Logger::Log("PatchWorker: gave up after timeout -- pattern(s) never resolved.");

        return 0;
    }
}

namespace PatchWorker
{
    void Start()
    {
        g_thread = CreateThread(nullptr, 0, &WorkerProc, nullptr, 0, nullptr);
        if (!g_thread)
            Logger::LogFormatted("PatchWorker: CreateThread failed, error=%lu", GetLastError());
    }

    void StopAndRevert()
    {
        g_stop.store(true, std::memory_order_relaxed);

        std::lock_guard<std::mutex> lock(g_mutex);
        DismountWeaponStripPatch::Remove();
        LongarmsStoreOnDismountHooks::Remove();

        if (g_minHookInitialized)
        {
            MH_Uninitialize();
            g_minHookInitialized = false;
        }

        if (g_thread)
        {
            CloseHandle(g_thread);
            g_thread = nullptr;
        }
    }
}
