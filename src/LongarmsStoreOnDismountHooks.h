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
// Neither function performs the actual weapon transfer -- they're just the
// bit accessor pair. The caller return address turned out useless (it
// points into ScriptHookRDR2's generic native dispatcher, not the real
// caller), so instead: GET's pattern happens to end exactly at the CALL
// opcode for the ped->WeaponComponent resolver both functions use (IDA:
// sub_140CC5C00). Install() decodes that CALL's rel32 to recover the
// resolver's real address without a separate signature, and every log line
// then calls it directly to print the live WeaponComponent+0x1B5 address
// for that ped -- feed that straight into a Cheat Engine/x64dbg Read
// breakpoint to find whatever actually consults the bit.
namespace LongarmsStoreOnDismountHooks
{
    bool Install();
    void Remove();
}
