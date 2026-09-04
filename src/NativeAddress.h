#pragma once
#include <cstdint>

// The one seam you're filling in with IDA: given a native hash, return the
// absolute address of its handler inside the running game process (whatever
// your scrEngine / native-registration-table walk resolves it to for this
// build). Return nullptr for anything not yet mapped -- NativeHook skips
// unresolved entries instead of hooking garbage.
namespace NativeAddress
{
    void* Resolve(uint64_t nativeHash);
}
