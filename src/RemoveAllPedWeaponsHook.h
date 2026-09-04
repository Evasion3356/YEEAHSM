#pragma once

// Hooks REMOVE_ALL_PED_WEAPONS_NATIVE (hash 0xF25DF915FA38C5F3 /
// WEAPON::REMOVE_ALL_PED_WEAPONS). Per IDA:
//   void __fastcall REMOVE_ALL_PED_WEAPONS(__int64 a1, unsigned __int8 a2, char a3)
// maps to script (ped, p1, p2).
namespace RemoveAllPedWeaponsHook
{
    bool Install();
    void Remove();
}
