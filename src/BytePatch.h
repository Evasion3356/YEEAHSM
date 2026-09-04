#pragma once
#include "PatternScan.h"
#include "Logger.h"

#include <windows.h>
#include <vector>
#include <cstdint>
#include <cstring>

// Applies/reverts a raw in-place code patch at an address found via AOB
// scan. Unlike NativeSigHook (a MinHook detour that intercepts a call),
// this directly overwrites instruction bytes -- use it when you want to
// change what an instruction does in place (e.g. neuter a conditional
// check) rather than redirect control flow to a detour.
//
// Not thread-safe against something executing at `address` at the exact
// instant Apply()/Revert() writes it -- fine for a toggle driven from our
// own script thread on a game whose relevant code isn't itself
// multithreaded at that granularity, but worth knowing if reused elsewhere.
class BytePatch
{
public:
    BytePatch(const char* name, const char* findPattern, std::vector<uint8_t> newBytes)
        : m_name(name), m_findPattern(findPattern), m_newBytes(std::move(newBytes))
    {
    }

    // Resolves the address and captures the original bytes. Does NOT apply
    // the patch -- call Apply() explicitly. Safe to call once at startup,
    // or repeatedly from a retry loop while waiting for the target process
    // to finish unpacking/deobfuscating itself -- pass quiet=true in that
    // case so a "not found yet" miss doesn't spam the log every attempt.
    bool Resolve(bool quiet = false)
    {
        m_address = PatternScan::Find(m_findPattern);
        if (!m_address)
        {
            if (!quiet)
                Logger::LogFormatted("%s: pattern not found -- game version mismatch or bad AOB.", m_name);
            return false;
        }

        auto* p = reinterpret_cast<uint8_t*>(m_address);
        m_original.assign(p, p + m_newBytes.size());

        Logger::LogFormatted("%s: resolved at 0x%p", m_name, m_address);
        return true;
    }

    bool Apply() { return Write(m_newBytes, true); }
    bool Revert() { return Write(m_original, false); }

    bool IsApplied() const { return m_applied; }
    void* GetAddress() const { return m_address; }

private:
    bool Write(const std::vector<uint8_t>& bytes, bool willBeApplied)
    {
        if (!m_address)
            return false;

        DWORD oldProtect;
        if (!VirtualProtect(m_address, bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            Logger::LogFormatted("%s: VirtualProtect failed, error=%lu", m_name, GetLastError());
            return false;
        }

        memcpy(m_address, bytes.data(), bytes.size());

        DWORD ignored;
        VirtualProtect(m_address, bytes.size(), oldProtect, &ignored);
        FlushInstructionCache(GetCurrentProcess(), m_address, bytes.size());

        m_applied = willBeApplied;
        return true;
    }

    const char* m_name;
    const char* m_findPattern;
    std::vector<uint8_t> m_newBytes;
    std::vector<uint8_t> m_original;
    void* m_address = nullptr;
    bool m_applied = false;
};
