#pragma once

// Hooks REMOVE_WEAPON_FROM_PED_NATIVE (hash 0x4899CB088EDF59B8 /
// WEAPON::REMOVE_WEAPON_FROM_PED). Per IDA:
//   void __fastcall REMOVE_WEAPON_FROM_PED(__int64 a1, unsigned int a2, __int64 a3, unsigned int a4)
// maps to script (ped, weaponHash, p2, removeReason).
namespace RemoveWeaponFromPedHook
{
    bool Install();
    void Remove();
}
