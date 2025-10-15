#include "MinHook.h"
#include "TweaksJIP.hpp"
#include "TweaksStewie.hpp"
#include "utils.hpp"
#include <cstdio>
#include <filesystem>
#include <nvse/PluginAPI.h>
#include <shared/Defines.hpp>
#include <shared/NVSEManager/NVSEGlobalManager.hpp>

#define VERSION 220

using NameInput_t = int(__fastcall *)(int a1, int a2, int a3);
NameInput_t OrigNameInput = nullptr;
int __fastcall HookNameInput(int a1, int a2, int a3)
{
    a3 = TranslateKeyToCP1251(a3);
    return OrigNameInput(a1, a2, a3);
}

bool InstallHooks()
{
#ifdef _DEBUG
    FILE *fDummy;
    AllocConsole();
    SetConsoleOutputCP(1251);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
#endif

    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));

    // Replaces 's' with 'в'
    Logger::GetInstance().Log("Patching plural\n");
    uintptr_t pluralChar = baseAddr + 0xC12018;
    SafeWrite8(pluralChar, 226);

    if (MH_Initialize() != MH_OK) return Logger::GetInstance().Log("Failed to initialize MinHook\n"), false;

    void *nameInputAddr = reinterpret_cast<void *>(baseAddr + 0x316B00);
    if (MH_CreateHook(nameInputAddr, &HookNameInput, reinterpret_cast<LPVOID *>(&OrigNameInput)) == MH_OK)
    {
        Logger::GetInstance().Log("NameInput hook created at 0x%p\n", nameInputAddr);
        if (MH_EnableHook(nameInputAddr) == MH_OK)
            Logger::GetInstance().Log("NameInput hook enabled successfully.\n");
        else
            Logger::GetInstance().Log("Failed to enable NameInput hook.\n");
    }
    else
        Logger::GetInstance().Log("Failed to create NameInput hook at 0x%p\n", nameInputAddr);

    char modulePath[MAX_PATH];
    GetModuleFileNameA(reinterpret_cast<HMODULE>(GetModuleHandle("RuLocaleFixes.dll")), modulePath, MAX_PATH);
    const std::string iniPath = std::filesystem::path(modulePath).parent_path().append("RuLocaleFixes.ini").string();

    bool enableFasterToCase = GetPrivateProfileIntA("Main", "EnableFasterToCase", 1, iniPath.c_str());
    bool enableJip = GetPrivateProfileIntA("Main", "EnableJIP", 1, iniPath.c_str());
    bool enableStewie = GetPrivateProfileIntA("Main", "EnableStewie", 1, iniPath.c_str());

    if (enableFasterToCase)
    {
        // Makes tolower and toupper faster
        Logger::GetInstance().Log("Patching toupper and tolower\n");

        // tolower
        // if just WriteRelJump - crash
        SafeWrite8(0xEC67B6, 0xEB);
        SafeWrite16(0xEC67C8, 0x9090);
        ReplaceCall(0xEC67CD, FastToLower);
        SafeWrite8(0xEC67D2, 0x90);

        // toupper
        // made similar to tolower, just in case, even though no crash with WriteRelJump
        // WriteRelJump(0xECA7F4, FastToUpper);
        SafeWrite8(0xECA800, 0xEB);
        SafeWrite16(0xECA812, 0x9090);
        ReplaceCall(0xECA817, FastToUpper);
        SafeWrite8(0xECA81C, 0x90);
    }

    if (enableJip) TweaksJIP::Apply();
    if (enableStewie) TweaksStewie::Apply();

    Logger::DestroyInstance();
    return true;
}

void MessageHandler(NVSEMessagingInterface::Message *msg)
{
    if (msg->type == NVSEMessagingInterface::kMessage_PostPostLoad) InstallHooks();
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface *nvse,
                                        PluginInfo *info)
{
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "RuLocaleFixes";
    info->version = VERSION;
    return true;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface *nvse)
{
    if (!nvse->isEditor)
    {
        NVSEGlobalManager &rNVSE = NVSEGlobalManager::GetSingleton();
        rNVSE.Initialize(nvse);
        rNVSE.RegisterPluginEventListener("NVSE", MessageHandler);
    }

    return true;
}

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
    return TRUE;
}
