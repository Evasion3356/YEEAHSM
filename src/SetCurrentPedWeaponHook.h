#pragma once

// Hooks SET_CURRENT_PED_WEAPON_NATIVE (hash 0xADF692B254977C0C /
// WEAPON::SET_CURRENT_PED_WEAPON). Per IDA:
//   void __fastcall SET_CURRENT_PED_WEAPON(__int64 a1, __int64 a2, __int64 a3,
//       __int64 a4, char a5, char a6)
// maps to script (ped, weaponHash, equipNow, attachPoint, p4, p5). NativeDB's
// attachPoint enum (eWeaponAttachPoint, natives.h) has no horse-related
// slot, so this native's job looks limited to body-worn slots -- hooking to
// confirm that against its real behavior rather than trust the header, same
// as we did for GIVE_WEAPON_TO_PED's float/int mismatch earlier.
namespace SetCurrentPedWeaponHook
{
    bool Install(bool quiet = false);
    void Remove();
}
