#pragma once

// Hooks DOES_ENTITY_EXIST_NATIVE (hash 0xD42BD6EB2E0F1677 /
// ENTITY::DOES_ENTITY_EXIST), found by AOB scan. Per IDA:
//   char __fastcall DOES_ENTITY_EXIST(int a1)
// -- a single Entity handle in, BOOL-as-char out. Simpler data point than
// GET_PLAYER_PED_NATIVE (one real script arg, no mystery second parameter)
// to help pin down whether these engine natives share a calling convention
// or each get a compiler-specialized signature based on arg count/usage.
namespace DoesEntityExistHook
{
    bool Install();
    void Remove();
}
