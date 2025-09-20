#pragma once

#include <windows.h>

#include <dbghelp.h>

class DbgSymbols
{
  public:
    DbgSymbols() = default;
    DbgSymbols(const char *moduleName);
    ~DbgSymbols();

    DWORD64 PrintSymbolsByAddr(DWORD64 offset) const;
    bool GetAddressByName(const char *funcName, DWORD64 &outAddr) const;

  private:
    HANDLE hSym = nullptr;
    HMODULE hModule = nullptr;
    DWORD64 baseAddr = 0;
    bool initialized = false;

    DWORD64 ResolveByName(const char *name) const;
};
