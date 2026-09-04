#pragma once
#include "PatternScan.h"
#include "Logger.h"

#include <MinHook.h>

// Generic single-native hook: resolves `pattern` via AOB scan, MH_CreateHook
// + MH_EnableHook's it, and hands back the original so a free-standing
// detour function can call through. One instance per native.
//
// Introduced once we had enough repeated hooks (GET_PLAYER_PED,
// DOES_ENTITY_EXIST, and now five weapon natives) that each having its own
// hand-rolled Install/Remove pair was pure duplication -- the actual
// per-native work is just "what's the signature" and "what do I log",
// which is exactly what varies between instantiations here.
template <typename Ret, typename... Args>
class NativeSigHook
{
public:
    using Fn = Ret (*)(Args...);

    NativeSigHook(const char* name, const char* pattern)
        : m_name(name), m_pattern(pattern)
    {
    }

    // quiet=true suppresses the "pattern not found" log -- for a retry loop
    // that expects misses while the target process is still unpacking.
    bool Install(Fn detour, bool quiet = false)
    {
        m_target = PatternScan::Find(m_pattern);
        if (!m_target)
        {
            if (!quiet)
                Logger::LogFormatted("%s: pattern not found -- game version mismatch or bad AOB.", m_name);
            return false;
        }

        MH_STATUS status = MH_CreateHook(m_target,
            reinterpret_cast<void*>(detour),
            reinterpret_cast<void**>(&m_original));
        if (status != MH_OK)
        {
            Logger::LogFormatted("%s: MH_CreateHook failed, status=%d", m_name, (int)status);
            m_target = nullptr;
            return false;
        }

        status = MH_EnableHook(m_target);
        if (status != MH_OK)
        {
            Logger::LogFormatted("%s: MH_EnableHook failed, status=%d", m_name, (int)status);
            MH_RemoveHook(m_target);
            m_target = nullptr;
            return false;
        }

        m_hooked = true;
        Logger::LogFormatted("%s: hooked at 0x%p", m_name, m_target);
        return true;
    }

    void Remove()
    {
        if (!m_hooked)
            return;
        MH_DisableHook(m_target);
        MH_RemoveHook(m_target);
        m_hooked = false;
        m_target = nullptr;
    }

    Ret CallOriginal(Args... args)
    {
        return m_original(args...);
    }

    // Address the pattern actually resolved to. Useful when a detour needs
    // to decode bytes near the hook site itself -- e.g. pulling the target
    // of a CALL instruction that sits at a known offset into the pattern,
    // to resolve an internal helper function without a separate signature.
    void* GetTarget() const
    {
        return m_target;
    }

private:
    const char* m_name;
    const char* m_pattern;
    void* m_target = nullptr;
    Fn m_original = nullptr;
    bool m_hooked = false;
};
