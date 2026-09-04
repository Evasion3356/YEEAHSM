#include "Logger.h"
#include "NativeHook.h"

#include <main.h>   // ScriptHookRDR2 SDK
#include <MinHook.h>
#include <windows.h>

namespace
{
    void ScriptMain()
    {
        Logger::Init();
        Logger::Log("NativeLogger script thread started. Press F9 to toggle logging.");

        if (MH_Initialize() != MH_OK)
        {
            Logger::Log("MH_Initialize failed -- aborting hook install.");
        }
        else
        {
            NativeHook::InstallAll();
        }

        while (true)
        {
            WAIT(0);
        }
    }

    void OnKeyboardMessage(DWORD key, WORD /*repeats*/, BYTE /*scanCode*/,
                            BOOL /*isExtended*/, BOOL /*isWithAlt*/,
                            BOOL wasDownBefore, BOOL isUpNow)
    {
        if (key == VK_F9 && !isUpNow && !wasDownBefore)
        {
            bool enabled = Logger::ToggleEnabled();
            Logger::Log(enabled ? "== Logging ENABLED ==" : "== Logging DISABLED ==");
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        keyboardHandlerRegister(OnKeyboardMessage);
        scriptRegister(hModule, ScriptMain);
        break;

    case DLL_PROCESS_DETACH:
        NativeHook::RemoveAll();
        MH_Uninitialize();
        keyboardHandlerUnregister(OnKeyboardMessage);
        scriptUnregister(hModule);
        Logger::Shutdown();
        break;
    }
    return TRUE;
}
