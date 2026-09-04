#include "PatternScan.h"
#include <windows.h>
#include <vector>
#include <cstdint>
#include <cstdlib>

namespace
{
    struct PatternByte
    {
        uint8_t value;
        bool wildcard;
    };

    std::vector<PatternByte> ParsePattern(const char* pattern)
    {
        std::vector<PatternByte> bytes;
        const char* p = pattern;
        while (*p)
        {
            while (*p == ' ')
                ++p;
            if (!*p)
                break;

            if (*p == '?')
            {
                bytes.push_back({ 0, true });
                ++p;
                if (*p == '?')
                    ++p; // tolerate "??" as a single wildcard token
            }
            else
            {
                bytes.push_back({ static_cast<uint8_t>(strtoul(p, nullptr, 16)), false });
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

        std::vector<PatternByte> bytes = ParsePattern(pattern);
        if (bytes.empty())
            return nullptr;

        auto start = reinterpret_cast<uint8_t*>(moduleBase);
        size_t patLen = bytes.size();
        if (moduleSize < patLen)
            return nullptr;

        for (size_t i = 0; i <= moduleSize - patLen; ++i)
        {
            bool matched = true;
            for (size_t j = 0; j < patLen; ++j)
            {
                if (bytes[j].wildcard)
                    continue;
                if (start[i + j] != bytes[j].value)
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
