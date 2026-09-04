#pragma once

// Hooks ScriptHookRDR2.dll's own exported native-invocation entry points:
// nativeInit / nativePush64 / nativeCall. Their addresses come straight from
// GetProcAddress on a documented, exported SDK function -- no IDA, no
// pattern scanning, nothing guessed. This exists to prove the MinHook +
// Logger pipeline actually works end-to-end on a "known good" target before
// NativeHook gets pointed at IDA-resolved, unverified internal engine
// addresses.
//
// Caveat: this only sees native calls made through the SDK's own invoke<>()
// path, i.e. calls this plugin makes itself -- not calls the game's own YSC
// scripts make internally. That's still NativeHook's job once
// NativeAddress::Resolve is implemented.
namespace SdkHook
{
    void InstallAll();
    void RemoveAll();
}
