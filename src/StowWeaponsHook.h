#pragma once

// Hooks sub_14089EE14 (IDA: StowWeapons), the function proven live (via a
// manual RETN patch) to be what actually moves a weapon onto the horse.
// Called directly as a normal internal function -- NOT through the
// native-dispatch table -- so unlike every hook before this one, the
// caller's return address is real and useful here, not a generic
// ScriptVM dispatcher address.
//
//   void __fastcall StowWeapons(__int64 a1, __int64 a2, unsigned int a3, __int64 a4, char a5)
//   pattern: 48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 45 33 F6 45 8B E0
//
// Logs a running call count, every argument, and the caller's return
// address on every call -- then deliberately does NOT call the original.
// Same effect as the manual RETN patch that first proved this is the real
// stow function, just applied via hook so it's toggleable/removable
// without touching the executable directly. Point of still logging: see
// how many distinct call sites reach this (dismount via sub_1409E2828,
// camp-arrival via some other path, maybe more) even while it's neutered.
namespace StowWeaponsHook
{
    bool Install(bool quiet = false);
    void Remove();
}
