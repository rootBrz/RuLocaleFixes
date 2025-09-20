#include "DbgSymbols.hpp"
#include "utils.hpp"
#include <cstring>

DbgSymbols::~DbgSymbols()
{
    if (initialized && hSym) SymCleanup(hSym);
}

DbgSymbols::DbgSymbols(const char *moduleName)
{
    hModule = GetModuleHandleA(moduleName);
    if (!hModule)
    {
        Logger::GetInstance().Log("Module %s not found!\n", moduleName);
        return;
    }

    char dllPath[MAX_PATH]{};
    if (!GetModuleFileNameA(hModule, dllPath, MAX_PATH))
    {
        Logger::GetInstance().Log("Failed to get module path\n");
        return;
    }

    char symbolDir[MAX_PATH]{};
    strcpy_s(symbolDir, dllPath);
    if (char *slash = strrchr(symbolDir, '\\')) *slash = '\0';

    static uintptr_t gSymUnique;
    hSym = reinterpret_cast<HANDLE>(&gSymUnique);

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    if (!SymInitialize(hSym, symbolDir, FALSE))
    {
        Logger::GetInstance().Log("SymInitialize failed (err=%lu)\n", GetLastError());
        return;
    }

    baseAddr = SymLoadModuleEx(hSym, nullptr, dllPath, nullptr, reinterpret_cast<DWORD64>(hModule), 0, nullptr, 0);

    if (!baseAddr)
    {
        Logger::GetInstance().Log("SymLoadModuleEx failed (err=%lu)\n", GetLastError());
        return;
    }

    initialized = true;
    return;
}

DWORD64 DbgSymbols::ResolveByName(const char *name) const
{
    SYMBOL_INFO_PACKAGE sip{};
    sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
    sip.si.MaxNameLen = MAX_SYM_NAME;

    if (!SymFromName(hSym, name, &sip.si))
        return 0;

    return sip.si.Address;
}

DWORD64 DbgSymbols::PrintSymbolsByAddr(DWORD64 offset) const
{
    SYMBOL_INFO_PACKAGE sip{};
    sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
    sip.si.MaxNameLen = MAX_SYM_NAME;

    DWORD64 displacement = 0;
    if (SymFromAddr(hSym, reinterpret_cast<DWORD64>(hModule) + offset, &displacement, &sip.si))
    {
        // Decorated name (raw from PDB)
        Logger::GetInstance().Log("Decorated: %s\n", sip.si.Name);

        // Demangled name (human readable)
        char undecorated[1024];
        if (UnDecorateSymbolName(sip.si.Name, undecorated, sizeof(undecorated), UNDNAME_COMPLETE))
        {
            Logger::GetInstance().Log("Demangled: %s\n", undecorated);
        }

        Logger::GetInstance().Log("Displacement: 0x%llx\n", displacement);
    }
    else
    {
        Logger::GetInstance().Log("SymFromAddr failed: %lu\n", GetLastError());
    }

    return 0;
}

bool DbgSymbols::GetAddressByName(const char *funcName, DWORD64 &outAddr) const
{
    outAddr = ResolveByName(funcName);
    if (outAddr)
    {
        Logger::GetInstance().Log("Func %s found at: 0x%p\n", funcName, reinterpret_cast<void *>(outAddr));
        return true;
    }
    Logger::GetInstance().Log("Func %s not found\n", funcName);
    return false;
}
