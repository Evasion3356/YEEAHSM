#include "Logger.h"
#include "PatchWorker.h"

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        Logger::Init();
        PatchWorker::Start();
        break;

    case DLL_PROCESS_DETACH:
        PatchWorker::StopAndRevert();
        Logger::Shutdown();
        break;
    }
    return TRUE;
}
