#include "PatternFinder.hpp"
#include "utils.hpp"
#include <Psapi.h>
#include <cstring>

namespace PatternFinder
{
Scanner::Scanner(const char *moduleName)
{
    hModule = GetModuleHandleA(moduleName);
    if (!hModule)
    {
        Logger::GetInstance().Log("Module %s not found!\n", moduleName);
        return;
    }

    MODULEINFO modInfo = {};
    if (GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
    {
        baseAddress = reinterpret_cast<std::uintptr_t>(modInfo.lpBaseOfDll);
        moduleSize = modInfo.SizeOfImage;
    }
}

void Scanner::ParsePattern(const std::string &pattern, std::vector<uint8_t> &bytes, std::string &mask)
{
    bytes.clear();
    mask.clear();

    size_t len = pattern.length();
    for (size_t i = 0; i < len;)
    {
        if (pattern[i] == ' ')
        {
            i++;
            continue;
        }

        if (pattern[i] == '?')
        {
            bytes.push_back(0);
            mask += '?';
            if (i + 1 < len && pattern[i + 1] == '?') i++;
            i++;
        }
        else
        {
            uint8_t byte = static_cast<uint8_t>(std::stoi(pattern.substr(i, 2), nullptr, 16));
            bytes.push_back(byte);
            mask += 'x';
            i += 2;
        }
    }
}

std::vector<std::uintptr_t> Scanner::FindPatternAll(const std::string &pattern)
{
    std::vector<std::uintptr_t> results;
    if (!hModule || !baseAddress || moduleSize == 0) return results;

    std::vector<uint8_t> patternBytes;
    std::string mask;
    ParsePattern(pattern, patternBytes, mask);

    std::uintptr_t end = baseAddress + moduleSize - patternBytes.size();
    uint8_t *basePtr = reinterpret_cast<uint8_t *>(baseAddress);

    for (std::uintptr_t i = 0; i <= end - baseAddress; ++i)
    {
        bool match = true;
        for (size_t j = 0; j < patternBytes.size(); ++j)
        {
            if (mask[j] == 'x' && basePtr[i + j] != patternBytes[j])
            {
                match = false;
                break;
            }
        }

        if (match) results.push_back(reinterpret_cast<std::uintptr_t>(&basePtr[i]));
    }

    return results;
}
} // namespace PatternFinder
