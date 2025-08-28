#include "nvse/PluginAPI.h"
#include <shared/NVSEManager/NVSEGlobalManager.hpp>
#include "MinHook.h"

#pragma comment(lib, "libMinHook.x86.lib")

static uint32_t TranslateKeyToCP1251(uint32_t key);

typedef int(__fastcall* NameInput)(int a1, int a2, int a3);
NameInput origNameInput = nullptr;

static int __fastcall hookedNameInput(int a1, int a2, int a3) {
    a3 = TranslateKeyToCP1251(a3);
    return origNameInput(a1, a2, a3);
}

static void InstallHooks()
{
    void* baseAddr = GetModuleHandle(NULL);

    // Add char convertion to name input if user's kb layout set to ru
    uintptr_t nameInputAddr = (uintptr_t)baseAddr + 0x316B00;
    MH_Initialize();
    MH_CreateHook((LPVOID)nameInputAddr, &hookedNameInput, reinterpret_cast<LPVOID*>(&origNameInput));
    MH_EnableHook((LPVOID)nameInputAddr);

    // Replaces 's' with 'в'
    uintptr_t pluralChar = (uintptr_t)baseAddr + 0xC12018;
    SafeWrite8(pluralChar, 226);
}

static void MessageHandler(NVSEMessagingInterface::Message* msg) {
	if (msg->type == NVSEMessagingInterface::kMessage_DeferredInit)
		InstallHooks();
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info) {
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "RuLocaleFixes";
	info->version = 100;
	return true;
}

EXTERN_DLL_EXPORT bool NVSEPlugin_Load(NVSEInterface* nvse) {
	if (!nvse->isEditor) {
		NVSEGlobalManager& rNVSE = NVSEGlobalManager::GetSingleton();
		rNVSE.Initialize(nvse);
		rNVSE.RegisterPluginEventListener("NVSE", MessageHandler);
	}

	return true;
}

BOOL WINAPI DllMain(
	HANDLE  hDllHandle,
	DWORD   dwReason,
	LPVOID  lpreserved
)
{
	return TRUE;
}

static uint32_t TranslateKeyToCP1251(uint32_t key) {
    HKL layout = GetKeyboardLayout(0);
    LANGID langId = PRIMARYLANGID(LOWORD(layout));

    if (langId != LANG_RUSSIAN || GetAsyncKeyState(VK_CONTROL) & 0x8000 || GetAsyncKeyState(VK_MENU) & 0x8000) {
        return key;
    }

    if (key <= 0x1F || key == 0x7F)
        return key;

    switch (key) {
    case 'a': return 0xF4; case 'b': return 0xE8; case 'c': return 0xF1; case 'd': return 0xE2;
    case 'e': return 0xF3; case 'f': return 0xE0; case 'g': return 0xEF; case 'h': return 0xF0;
    case 'i': return 0xF8; case 'j': return 0xEE; case 'k': return 0xEB; case 'l': return 0xE4;
    case 'm': return 0xFC; case 'n': return 0xF2; case 'o': return 0xF9; case 'p': return 0xE7;
    case 'q': return 0xE9; case 'r': return 0xEA; case 's': return 0xFB; case 't': return 0xE5;
    case 'u': return 0xE3; case 'v': return 0xEC; case 'w': return 0xF6; case 'x': return 0xF7;
    case 'y': return 0xED; case 'z': return 0xFF;

    case 'A': return 0xD4; case 'B': return 0xC8; case 'C': return 0xD1; case 'D': return 0xC2;
    case 'E': return 0xD3; case 'F': return 0xC0; case 'G': return 0xCF; case 'H': return 0xD0;
    case 'I': return 0xD8; case 'J': return 0xCE; case 'K': return 0xCB; case 'L': return 0xC4;
    case 'M': return 0xDC; case 'N': return 0xD2; case 'O': return 0xD9; case 'P': return 0xC7;
    case 'Q': return 0xC9; case 'R': return 0xCA; case 'S': return 0xDB; case 'T': return 0xC5;
    case 'U': return 0xC3; case 'V': return 0xCC; case 'W': return 0xD6; case 'X': return 0xD7;
    case 'Y': return 0xCD; case 'Z': return 0xDF;

    case ',': return 0xE1; case '.': return 0xFE;
    case '<': return 0xC1; case '>': return 0xDE;
    case ';': return 0xE6; case ':': return 0xC6;
    case '\'': return 0xFD; case '"': return 0xDD;
    case '[': return 0xF5; case ']': return 0xFA;
    case '{': return 0xD5; case '}': return 0xDA;
    }

    return key;
}
