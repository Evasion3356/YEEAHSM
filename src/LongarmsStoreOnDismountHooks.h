#pragma once

// Hooks the confirmed-correct GET/SET pair for the "store longarm on horse
// on dismount" bit, verified in IDA against WeaponComponent+0x1B5 (== +437
// decimal, same offset both functions touch via sub_140CC5C00(a1)):
//
//   char GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT(__int64 a1, char a2)
//     -- 0x5A695BD328586B44, pattern: 40 53 48 83 EC ? 8A DA 48 85 C9 74 ? E8
//   void SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT(__int64 a1, char a2, char a3)
//     -- 0xB832F1A686B9B810, pattern: 48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC ? 8B 81 ? ? ? ? 41 8A D8
//
// EXPERIMENT (not a fix -- diagnostic): forces SET's storeLongarms argument
// to 0 on every call regardless of what was actually requested (so the real
// underlying byte gets cleared no matter who's trying to set it), and
// forces GET's return value to 0 regardless of the real byte state. Neither
// native ever showed up in any decompiled script (full-repo search across
// both script sets came back empty), so this can't be gated by caller --
// it's unconditional for as long as this hook is installed.
//
// Point of the test: confirm whether the camp-arrival weapon relocation
// depends on this same flag. If forcing it to 0 also stops weapons moving
// to the horse when entering camp, that's real evidence of a shared
// mechanism. If camp behavior is unaffected, camp uses something else
// entirely.
namespace LongarmsStoreOnDismountHooks
{
    bool Install(bool quiet = false);
    void Remove();
}
