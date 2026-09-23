#include "Logger.h"
#include "PatchWorker.h"

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        Logger::Init();
        PatchWorker::Start();
        break;

    case DLL_PROCESS_DETACH:
        // Non-null lpReserved: the process is exiting and every other
        // thread is already gone -- possibly the patch worker while it held
        // PatchWorker's mutex, which StopAndRevert() would then wait on
        // forever. Unhooking is pointless at that point anyway.
        if (lpReserved)
            break;
        PatchWorker::StopAndRevert();
        Logger::Shutdown();
        break;
    }
    return TRUE;
}
