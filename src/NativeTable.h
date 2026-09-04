#pragma once
#include <cstdint>

struct NativeInfo
{
    uint64_t hash;
    const char* name;
};

// Hashes verified directly against ScriptHookSDK/inc/natives.h (build b1207)
// shipped in this repo -- not guessed, not pulled from an external DB. Weapon
// natives most likely to fire around horse mount/dismount, plus a couple of
// equip/ammo natives for context. Extend freely; NativeHook::InstallAll()
// just walks this table.
static const NativeInfo kNativesOfInterest[] = {
    { 0x5E3BDDBCB83F3D84, "GIVE_WEAPON_TO_PED" },
    { 0x94A3C1B804D291EC, "HOLSTER_PED_WEAPONS" },
    { 0xFCCC886EDE3C63EC, "HIDE_PED_WEAPONS" },
    { 0xF25DF915FA38C5F3, "REMOVE_ALL_PED_WEAPONS" },
    { 0x4899CB088EDF59B8, "REMOVE_WEAPON_FROM_PED" },
    { 0x51C3B71591811485, "REMOVE_WEAPON_FROM_PED_BY_GUID" },
    { 0xADF692B254977C0C, "SET_CURRENT_PED_WEAPON" },
    { 0x3A87E44BB9A01D54, "GET_CURRENT_PED_WEAPON" },
    { 0x14E56BC5B5DB6A19, "SET_PED_AMMO" },
};
