#include "PatchWorker.h"
#include "StowWeaponsHook.h"
#include "Logger.h"

#include <windows.h>
#include <MinHook.h>
#include <atomic>

namespace
{
    constexpr DWORD kRetryIntervalMs = 500;
    constexpr DWORD kTimeoutMs = 120000; // 2 minutes
    constexpr DWORD kStatusEveryMs = 10000; // reassurance log every ~10s

    std::atomic<bool> g_stop{ false };

    // Raw SRWLOCK instead of std::mutex: std::mutex's lock()/unlock() can
    // throw std::system_error, which drags the whole C++ exception-unwinder
    // (and <system_error>'s string tables) into the binary even though it
    // never actually throws here. SRWLOCK is a plain WinAPI primitive with
    // no such dependency and needs no destructor.
    SRWLOCK g_lock = SRWLOCK_INIT; // guards the Install()/Remove() race
                                    // between the worker thread and
                                    // StopAndRevert()

    struct ScopedLock
    {
        SRWLOCK& lock;
        explicit ScopedLock(SRWLOCK& l) : lock(l) { AcquireSRWLockExclusive(&lock); }
        ~ScopedLock() { ReleaseSRWLockExclusive(&lock); }
    };

    HANDLE g_thread = nullptr;

    bool g_stowHookInstalled = false;
    bool g_minHookInitialized = false;

    DWORD WINAPI WorkerProc(LPVOID)
    {
        if (MH_Initialize() == MH_OK)
        {
            g_minHookInitialized = true;
        }
        else
        {
            Logger::Log("PatchWorker: MH_Initialize failed -- StowWeaponsHook can't be installed.");
        }

        DWORD elapsed = 0;
        DWORD sinceStatus = 0;

        while (!g_stop.load(std::memory_order_relaxed) && elapsed < kTimeoutMs)
        {
            bool allDone;
            {
                ScopedLock lock(g_lock);
                if (g_stop.load(std::memory_order_relaxed))
                    break;

                if (!g_stowHookInstalled && g_minHookInitialized)
                    g_stowHookInstalled = StowWeaponsHook::Install(/*quiet=*/true);

                allDone = g_stowHookInstalled || !g_minHookInitialized;
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

        ScopedLock lock(g_lock);
        StowWeaponsHook::Remove();

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
