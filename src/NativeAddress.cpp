#include "NativeAddress.h"

// TODO(you): implement via your IDA-derived native registration table walk.
// Until this returns real addresses, NativeHook::InstallAll() will log
// "skip (unresolved)" for every table entry and hook nothing -- that's the
// scaffold behaving correctly, not a bug.
namespace NativeAddress
{
    void* Resolve(uint64_t nativeHash)
    {
        (void)nativeHash;
        return nullptr;
    }
}
