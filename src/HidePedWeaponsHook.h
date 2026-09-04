#pragma once

// Hooks HIDE_PED_WEAPONS_NATIVE (hash 0xFCCC886EDE3C63EC /
// WEAPON::HIDE_PED_WEAPONS_). Per IDA:
//   void __fastcall HIDE_PED_WEAPONS(__int64 a1, int a2, char a3)
// maps to script (ped, p0, immediately).
//
// Re-enabled to investigate why entering camp still relocates the weapon
// to the horse even with the dismount-gate patch (DismountWeaponStripPatch)
// applied -- player_camp.c's func_14() calls this directly
// (WEAPON::_HIDE_PED_WEAPONS(Global_33, 2, true)) as part of the camp's
// "force unarmed" step, independent of the mount/dismount system. Logging
// its args/caller here to see whether it's the trigger for the relocation
// or just an unrelated side effect.
namespace HidePedWeaponsHook
{
    bool Install(bool quiet = false);
    void Remove();
}
