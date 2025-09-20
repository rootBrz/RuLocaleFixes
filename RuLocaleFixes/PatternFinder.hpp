#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace PatternFinder
{
class Scanner
{
  public:
    Scanner() = default;
    Scanner(const char *moduleName);

    // Pattern format: "48 8B ?? ?? ?? 48 85 C0"
    std::vector<std::uintptr_t> FindPatternAll(const std::string &pattern);

  private:
    void ParsePattern(const std::string &pattern, std::vector<uint8_t> &bytes, std::string &mask);

    HMODULE hModule = nullptr;
    std::uintptr_t baseAddress = 0;
    std::size_t moduleSize = 0;
};
} // namespace PatternFinder
