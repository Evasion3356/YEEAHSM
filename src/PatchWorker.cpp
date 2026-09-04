#include "PatchWorker.h"
#include "DismountWeaponStripPatch.h"
#include "Logger.h"

#include <windows.h>
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

    DWORD WINAPI WorkerProc(LPVOID)
    {
        DWORD elapsed = 0;
        DWORD sinceStatus = 0;

        while (!g_stop.load(std::memory_order_relaxed) && elapsed < kTimeoutMs)
        {
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                if (g_stop.load(std::memory_order_relaxed))
                    break;
                if (DismountWeaponStripPatch::Install(/*quiet=*/true))
                    return 0; // applied, done
            }

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
            Logger::Log("PatchWorker: gave up after timeout -- pattern never resolved.");

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

        if (g_thread)
        {
            CloseHandle(g_thread);
            g_thread = nullptr;
        }
    }
}
