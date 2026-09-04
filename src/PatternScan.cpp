#include "PatternScan.h"
#include <windows.h>
#include <cstdint>

namespace
{
    struct PatternByte
    {
        uint8_t value;
        bool wildcard;
    };

    // Hand-rolled in place of strtoul: UCRT's strtoul is locale-aware (it
    // consults ctype/codepage tables to classify digits) and shares its
    // parser with strtod/strtof, which drags in floating-point conversion
    // tables neither AOB scanning nor this hex-byte parse ever needs. We
    // only ever parse two hex nibbles at a time, so do that directly.
    uint8_t ParseHexByte(const char* p)
    {
        auto nibble = [](char c) -> uint8_t
        {
            if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
            if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
            if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
            return 0;
        };
        uint8_t value = nibble(p[0]);
        if (p[1] && p[1] != ' ')
            value = static_cast<uint8_t>((value << 4) | nibble(p[1]));
        return value;
    }

    // Fixed-capacity in place of std::vector: every AOB pattern in this repo
    // is well under 64 bytes, and a stack array means ParsePattern never
    // touches the heap or exception machinery (no bad_alloc/length_error
    // paths for the linker to pull in the C++ unwinder for).
    constexpr size_t kMaxPatternBytes = 64;

    struct PatternBytes
    {
        PatternByte data[kMaxPatternBytes];
        size_t count = 0;
    };

    PatternBytes ParsePattern(const char* pattern)
    {
        PatternBytes bytes;
        const char* p = pattern;
        while (*p && bytes.count < kMaxPatternBytes)
        {
            while (*p == ' ')
                ++p;
            if (!*p)
                break;

            if (*p == '?')
            {
                bytes.data[bytes.count++] = { 0, true };
                ++p;
                if (*p == '?')
                    ++p; // tolerate "??" as a single wildcard token
            }
            else
            {
                bytes.data[bytes.count++] = { ParseHexByte(p), false };
                while (*p && *p != ' ')
                    ++p;
            }
        }
        return bytes;
    }
}

namespace PatternScan
{
    bool GetMainModuleRange(void** outBase, size_t* outSize)
    {
        HMODULE hModule = GetModuleHandleA(nullptr);
        if (!hModule)
            return false;

        auto base = reinterpret_cast<uint8_t*>(hModule);
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            return false;

        auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE)
            return false;

        *outBase = base;
        *outSize = nt->OptionalHeader.SizeOfImage;
        return true;
    }

    void* Find(const char* pattern, void* moduleBase, size_t moduleSize)
    {
        if (!moduleBase || !moduleSize)
        {
            void* base = nullptr;
            size_t size = 0;
            if (!GetMainModuleRange(&base, &size))
                return nullptr;
            moduleBase = base;
            moduleSize = size;
        }

        PatternBytes bytes = ParsePattern(pattern);
        if (bytes.count == 0)
            return nullptr;

        auto start = reinterpret_cast<uint8_t*>(moduleBase);
        size_t patLen = bytes.count;
        if (moduleSize < patLen)
            return nullptr;

        for (size_t i = 0; i <= moduleSize - patLen; ++i)
        {
            bool matched = true;
            for (size_t j = 0; j < patLen; ++j)
            {
                if (bytes.data[j].wildcard)
                    continue;
                if (start[i + j] != bytes.data[j].value)
                {
                    matched = false;
                    break;
                }
            }
            if (matched)
                return start + i;
        }

        return nullptr;
    }

    size_t ToModuleOffset(void* address)
    {
        void* base = nullptr;
        size_t size = 0;
        if (!GetMainModuleRange(&base, &size))
            return 0;

        auto addr = reinterpret_cast<uint8_t*>(address);
        auto baseAddr = reinterpret_cast<uint8_t*>(base);
        if (addr < baseAddr || addr >= baseAddr + size)
            return 0; // not within the main module

        return static_cast<size_t>(addr - baseAddr);
    }
}
