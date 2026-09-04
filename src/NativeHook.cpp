#include "NativeHook.h"
#include "NativeTable.h"
#include "NativeAddress.h"
#include "Logger.h"

#include <MinHook.h>
#include <array>
#include <utility>
#include <cstddef>
#include <intrin.h>

// ASSUMPTION, not verified fact: RAGE native handlers across GTA4/5/RDR2 are
// conventionally void(*)(rage::scrNativeCallContext*) -- a single pointer in,
// void return. This scaffold deliberately does NOT decode that context
// struct (offsets are build-specific and guessing them risks reading garbage
// or corrupting the stack) -- it only logs the hash, the raw context
// pointer, and the caller's return address. That alone answers "does this
// fire, and from where" without needing the struct layout. Confirm the
// single-pointer/void-return shape against what you resolve in IDA before
// trusting this blindly; if a target native turns out to use a different
// convention, DetourThunk below needs to match it or you'll corrupt state.

namespace
{
    constexpr size_t kCount = std::size(kNativesOfInterest);

    void* g_target[kCount] = {};
    void* g_original[kCount] = {};
    bool  g_hooked[kCount] = {};

    void OnCall(size_t index, void* ctx, void* returnAddress)
    {
        const NativeInfo& info = kNativesOfInterest[index];

        if (Logger::IsEnabled())
        {
            Logger::LogFormatted(
                "native fired: %-32s hash=0x%016llX ctx=0x%p retaddr=0x%p",
                info.name, info.hash, ctx, returnAddress);
        }

        using OriginalFn = void(*)(void*);
        auto original = reinterpret_cast<OriginalFn>(g_original[index]);
        if (original)
            original(ctx);
    }

    template <size_t Index>
    void DetourThunk(void* ctx)
    {
        OnCall(Index, ctx, _ReturnAddress());
    }

    template <size_t... Is>
    constexpr std::array<void(*)(void*), sizeof...(Is)> MakeThunkTable(std::index_sequence<Is...>)
    {
        return { &DetourThunk<Is>... };
    }

    constexpr auto kThunks = MakeThunkTable(std::make_index_sequence<kCount>{});
}

namespace NativeHook
{
    void InstallAll()
    {
        for (size_t i = 0; i < kCount; ++i)
        {
            const NativeInfo& info = kNativesOfInterest[i];

            void* target = NativeAddress::Resolve(info.hash);
            if (!target)
            {
                Logger::LogFormatted("skip (unresolved): %-32s hash=0x%016llX", info.name, info.hash);
                continue;
            }

            void* detour = reinterpret_cast<void*>(kThunks[i]);
            MH_STATUS status = MH_CreateHook(target, detour, &g_original[i]);
            if (status != MH_OK)
            {
                Logger::LogFormatted("MH_CreateHook failed: %-32s status=%d", info.name, (int)status);
                continue;
            }

            status = MH_EnableHook(target);
            if (status != MH_OK)
            {
                Logger::LogFormatted("MH_EnableHook failed: %-32s status=%d", info.name, (int)status);
                MH_RemoveHook(target);
                continue;
            }

            g_target[i] = target;
            g_hooked[i] = true;
            Logger::LogFormatted("hooked: %-32s hash=0x%016llX addr=0x%p", info.name, info.hash, target);
        }
    }

    void RemoveAll()
    {
        for (size_t i = 0; i < kCount; ++i)
        {
            if (!g_hooked[i])
                continue;

            MH_DisableHook(g_target[i]);
            MH_RemoveHook(g_target[i]);
            g_hooked[i] = false;
        }
    }
}
