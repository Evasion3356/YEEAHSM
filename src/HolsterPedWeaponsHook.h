#pragma once

// Hooks HOLSTER_PED_WEAPONS_NATIVE (hash 0x94A3C1B804D291EC /
// WEAPON::HOLSTER_PED_WEAPONS_). Per IDA:
//   void __fastcall HOLSTER_PED_WEAPONS(__int64 a1, char a2, char a3, char a4, char a5)
// maps to script (ped, p1, p2, p3, immediately).
namespace HolsterPedWeaponsHook
{
    bool Install();
    void Remove();
}
