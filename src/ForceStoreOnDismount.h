#pragma once

// Repeatedly calls WEAPON::_SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT(
//     PLAYER_PED_ID(), true, 0)
// (hash 0xB832F1A686B9B810) via the SDK's normal invoke<>() path -- NOT a
// hook, just a direct call, same as any script would make. Confirmed in IDA
// to do bit manipulation on WeaponComponent+0x1B5, paired with the getter
// _GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT (0x5A695BD328586B44) that reads
// the same offset. No YSC script actually calls this setter (checked --
// zero hits across the decompiled 1491.50 scripts), so whatever sets this
// bit in practice does it from engine code, or script data we haven't
// found; calling it ourselves is just to force the bit on reliably.
//
// NOTE: an earlier version of this forced hash 0x641351E9AD103890 instead --
// that was wrong. It was inferred purely from being called in the same tick
// as player_horse.c's saddle-weapon eligibility check, not from actually
// verifying its behavior. Its real signature only takes 2 args (ped, bool),
// not the 3 args (ped, bool, int) _SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT
// takes, so it can't be the same native under a different name -- it's
// something else entirely, still unidentified.
//
// Purpose: the WeaponComponent+0x1B5 bit likely only gets written by the
// game in narrow windows. Forcing it true on every tick removes that
// flakiness so a write breakpoint on that offset in IDA/x64dbg hits
// reliably instead of intermittently -- this is purely a debugging aid,
// not part of the eventual mod.
namespace ForceStoreOnDismount
{
    // Called every ScriptMain tick. No-ops unless enabled via F10.
    void Tick();

    bool Toggle();
    bool IsEnabled();
}
