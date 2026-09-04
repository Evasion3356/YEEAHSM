#pragma once

// Hooks HIDE_PED_WEAPONS_NATIVE (hash 0xFCCC886EDE3C63EC /
// WEAPON::HIDE_PED_WEAPONS_). Per IDA:
//   void __fastcall HIDE_PED_WEAPONS(__int64 a1, int a2, char a3)
// maps to script (ped, p0, immediately).
namespace HidePedWeaponsHook
{
    bool Install();
    void Remove();
}
