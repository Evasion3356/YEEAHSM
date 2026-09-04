#include "Logger.h"
#include "PatchWorker.h"

#include <windows.h>

// extern "C" so the symbol is emitted undecorated as "DllMain" -- Release/
// Analyize point the linker's raw PE entry point straight at it via
// /ENTRY:DllMain, skipping the CRT startup thunk (_DllMainCRTStartup) and
// the ~25KB of locale/argv/mbcs init it otherwise unconditionally pulls in.
// Safe here because every static object in this DLL is constant-initialized
// (see NativeSigHook's constexpr ctor) -- there is no CRT dynamic-init pass
// to skip.
extern "C" BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD reason, LPVOID)
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
