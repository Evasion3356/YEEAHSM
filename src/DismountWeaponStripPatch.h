#pragma once

// Patches the master "should we strip weapons on this mount/dismount
// transition" read inside sub_1409E2828 (Research/StrangeFunction.txt),
// found live at RDR2.exe+9E29A6:
//
//   8A 86 8D010000   mov al,[rsi+0x18D]   <- v26 = *(BYTE*)(v13+397)
//   A8 06            test al,6
//   75 1C            jne +0x1E
//
// v98 = (v26 & 6) != 0 || (!a9 && (v26 & 8) != 0) gates the entire
// weapon-relocation block later in the function. Patches the 6-byte `mov`
// to `xor al,al` + 4 NOPs (same effect as the manual byte-zero that
// confirmed this location, just resolved via AOB scan instead of a
// hardcoded address) -- the following test/jne execute unmodified against
// an always-zero AL, so v98 is always false and the strip never runs.
namespace DismountWeaponStripPatch
{
    // Resolves the address and applies the patch. Returns false (and logs
    // why, unless quiet=true) if the pattern wasn't found -- doesn't throw
    // or crash. quiet=true is for PatchWorker's retry loop, where a miss is
    // expected and shouldn't spam the log every attempt.
    bool Install(bool quiet = false);

    // Reverts to stock bytes if applied. Call from DllMain on
    // DLL_PROCESS_DETACH so unloading the ASI always leaves the game clean.
    void Remove();
}
