#pragma once

namespace NativeHook
{
    // Resolves + MH_CreateHook's every entry in kNativesOfInterest, logging
    // pass/fail per entry. Safe to call before NativeAddress::Resolve is
    // implemented -- unresolved entries are just skipped.
    void InstallAll();

    // Disables + removes every hook InstallAll() actually installed. Call
    // from DLL_PROCESS_DETACH before MH_Uninitialize.
    void RemoveAll();
}
