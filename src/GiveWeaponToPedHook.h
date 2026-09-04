#pragma once

// Hooks GIVE_WEAPON_TO_PED_NATIVE (hash 0x5E3BDDBCB83F3D84 /
// WEAPON::GIVE_WEAPON_TO_PED). Per IDA:
//   __int64 __fastcall GIVE_WEAPON_TO_PED(__int64 a1, int a2, int a3, char a4,
//       char a5, int a6, char a7, int a8, int a9, int a10, char a11, int a12, char a13)
// maps to script (ped, weaponHash, ammoCount, bForceInHand, bForceInHolster,
// attachPoint, bAllowMultipleCopies, p7, p8, addReason, bIgnoreUnlocks,
// permanentDegradation, p12). p7/p8/permanentDegradation show up as plain
// `int` in Hex-Rays' decompile, but that's a Hex-Rays typing artifact, not
// the real ScriptVM contract -- NativeDB types them float, which is what
// actually gets pushed, so the detour bit-reinterprets those three 32-bit
// slots as float before logging rather than trusting the decompiled type.
namespace GiveWeaponToPedHook
{
    bool Install();
    void Remove();
}
