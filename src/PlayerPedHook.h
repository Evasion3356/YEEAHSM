#pragma once

// Hooks GET_PLAYER_PED_NATIVE (hash 0x275F255ED201B937 / script native
// GET_PLAYER_PED) directly at its internal handler address, found by AOB
// scan rather than a registration-table walk. First real (non-SDK-export)
// engine hook -- exists to validate PatternScan + MinHook against a target
// that is trivial to sanity-check: called every frame, and for player 0
// should always return the same non-zero ped handle.
//
// IMPORTANT: the real handler signature here is
//   __int64 (__fastcall*)(__int64 a1, __int64 a2)
// per direct IDA decompilation -- NOT the generic void(void*) context-ptr
// shape NativeHook.cpp assumed for the rest of the (still-unresolved)
// native table. Don't reuse that assumption elsewhere without re-verifying
// per native; this one data point suggests simple/low-arg natives may get
// a specialized signature rather than a single uniform one.
namespace PlayerPedHook
{
    bool Install();
    void Remove();
}
