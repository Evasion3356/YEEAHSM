#pragma once
#include <cstddef>

// Minimal IDA-style AOB pattern scanner. No dependency beyond kernel32 --
// reads the PE header of the running module directly instead of pulling in
// psapi/toolhelp.
namespace PatternScan
{
    // Resolves the main module's (the game .exe) base address and
    // SizeOfImage from its own PE header. Used as the default scan range
    // for Find() when no explicit range is given.
    bool GetMainModuleRange(void** outBase, size_t* outSize);

    // Scans [moduleBase, moduleBase+moduleSize) for `pattern`, e.g.
    // "40 53 48 83 EC ? 33 DB 81 F9" ('?' = wildcard byte, matches
    // anything). Returns the address of the first match, or nullptr.
    // Pass moduleBase=nullptr to scan the main module automatically.
    void* Find(const char* pattern, void* moduleBase = nullptr, size_t moduleSize = 0);
}
