#include "TweaksStewie.hpp"
#include "DbgSymbols.hpp"
#include "PatternFinder.hpp"
#include "shared/SafeWrite/SafeWrite.hpp"
#include "utils.hpp"
#include <cstdint>

using InputHandleKey_t = bool(__fastcall *)(void *thisptr, void *edx, unsigned int key);
using MenuHandleKey_t = char(__fastcall *)(void *thisptr, void *edx, unsigned int key);
using AddTweaksButton_t = void(__fastcall *)(void *startMenuOptions, void *edx, void **settingsMenuItem);

InputHandleKey_t OrigInputHandleKey = nullptr;
MenuHandleKey_t OrigMenuHandleKey = nullptr;
AddTweaksButton_t OrigAddTweaksButton = nullptr;

bool __fastcall HookInputHandleKey(void *thisptr, void *edx, unsigned int key)
{
    key = TranslateKeyToCP1251(key);
    return OrigInputHandleKey(thisptr, edx, key);
}

char __fastcall HookMenuHandleKey(void *thisptr, void *edx, unsigned int key)
{
    key = TranslateKeyToCP1251(key);
    return OrigMenuHandleKey(thisptr, edx, key);
}

void __fastcall HookAddTweaksButton(void *startMenuOptions, void *edx, void **settingsMenuItem)
{
    OrigAddTweaksButton(startMenuOptions, edx, settingsMenuItem);
    uintptr_t *arrayStruct = reinterpret_cast<uintptr_t *>(startMenuOptions);
    uintptr_t **menuItems = reinterpret_cast<uintptr_t **>(arrayStruct[1]);
    uintptr_t itemCount = arrayStruct[2];
    uintptr_t *tweaksButton = reinterpret_cast<uintptr_t *>(menuItems[itemCount - 1]); // Last because of append
    if (!tweaksButton) return;

    char *displayString = reinterpret_cast<char *>(tweaksButton[1]);
    if (displayString && strcmp(displayString, "Tweaks") != 0) return;

    static uint8_t tweaksText[] = {0xD2, 0xE2, 0xE8, 0xEA, 0xE8, 0x00};
    tweaksButton[1] = reinterpret_cast<uintptr_t>(tweaksText); // "Tweaks" -> "Твики"
}

void *origJsonGetString = nullptr;
void *savedRetAddr = nullptr;
void __declspec(naked) HookJsonGetString()
{
    __asm {
        push eax
        mov eax, [esp + 4]
        mov savedRetAddr, eax
        pop eax

        mov dword ptr [esp], offset PostHandler
        jmp dword ptr [origJsonGetString]

    PostHandler:
        pushad
        pushfd
        push eax
        call ConvertStdStringUtf8ToCp1251
        add esp, 4
        popfd
        popad
        jmp dword ptr [savedRetAddr]
    }
}

namespace TweaksStewie
{
void Apply()
{
    if (!GetModuleHandle("nvse_stewie_tweaks.dll")) return;

    DbgSymbols resolver("nvse_stewie_tweaks.dll");
    PatternFinder::Scanner search("nvse_stewie_tweaks.dll");

    ResolveAndHook(resolver, "MenuSearch::SearchBar::HandleKey", HookMenuHandleKey, reinterpret_cast<void **>(&OrigMenuHandleKey));
    ResolveAndHook(resolver, "InputField::HandleKey", HookInputHandleKey, reinterpret_cast<void **>(&OrigInputHandleKey));
    ResolveAndHook(resolver, "nlohmann::basic_json<std::map,std::vector,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,bool,__int64,unsigned __int64,double,std::allocator,nlohmann::adl_serializer,std::vector<unsigned char,std::allocator<unsigned char> > >::get<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,0>", HookJsonGetString, reinterpret_cast<void **>(&origJsonGetString));
    ResolveAndHook(resolver, "addTweaksButton", HookAddTweaksButton, reinterpret_cast<void **>(&OrigAddTweaksButton));

    DWORD64 toLowerAddr = NULL;
    resolver.GetAddressByName("tolower", toLowerAddr);
    if (toLowerAddr) WriteRelJump(static_cast<uintptr_t>(toLowerAddr), FastToLower);

    auto patchBytes = [&search](const std::string &pattern, const void *replacement, size_t size)
    {
        for (uintptr_t &addr : search.FindPatternAll(pattern))
        {
            if (!addr) continue;

            SafeWriteBuf(addr, const_cast<void *>(replacement), size);
            char *string = reinterpret_cast<char *>(calloc(size + 1, 1));
            bool skipFirst = *reinterpret_cast<const uint8_t *>(replacement) == 0;
            memcpy(string, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(replacement) + skipFirst), size - skipFirst);
            string[size] = '\0';
            Logger::GetInstance().Log("Patched 0x%p to %s\n", addr, string);
            free(string);
        }
    };

    constexpr uint8_t allBytes[] = {0x00, 0xC2, 0xF1, 0xE5, 0x00};                                                                                                           // Все
    constexpr uint8_t notBytes[] = {0xC1, 0xC5, 0xC7, 0x20};                                                                                                                 // "БЕЗ "
    constexpr uint8_t tweaksSettingsBytes[] = {0x00, 0xD2, 0xE2, 0xE8, 0xEA, 0xE8, 0x3a, 0x20, 0x25, 0x73, 0x00};                                                            // "Твики: %s"
    constexpr uint8_t otherCategoryBytes[] = {0x00, 0xC8, 0xED, 0xEE, 0xE5, 0x00};                                                                                           // "Иное"
    constexpr uint8_t dontShowAgainBytes[] = {0xC1, 0xEE, 0xEB, 0xFC, 0xF8, 0xE5, 0x20, 0xED, 0xE5, 0x20, 0xEF, 0xEE, 0xEA, 0xE0, 0xE7, 0xFB, 0xE2, 0xE0, 0xF2, 0xFC, 0x00}; // Больше не показывать
    patchBytes("00 41 6C 6C 00", allBytes, sizeof(allBytes));                                                                                                                // All
    patchBytes("4E 4F 54 20", notBytes, 4);                                                                                                                                  // "NOT "
    patchBytes("00 54 77 65 61 6B 73 20 25 73 00", tweaksSettingsBytes, sizeof(tweaksSettingsBytes));                                                                        // "Tweaks %s"
    patchBytes("00 4F 74 68 65 72 00", otherCategoryBytes, sizeof(otherCategoryBytes));                                                                                      // "Other"
    patchBytes("44 6F 6E 27 74 20 73 68 6F 77 20 74 68 69 73 20 61 67 61 69 6E 00", dontShowAgainBytes, sizeof(dontShowAgainBytes));                                         // Don't show this again

    // Необходим перезапуск для применения изменений!
    constexpr uint8_t restartPromptBytes[] = {0xCD, 0xE5, 0xEE, 0xE1, 0xF5, 0xEE, 0xE4, 0xE8, 0xEC, 0x20, 0xEF, 0xE5, 0xF0, 0xE5, 0xE7, 0xE0, 0xEF, 0xF3, 0xF1, 0xEA, 0x20, 0xE4, 0xEB, 0xFF, 0x20, 0xEF, 0xF0, 0xE8, 0xEC, 0xE5, 0xED, 0xE5, 0xED, 0xE8, 0xFF, 0x20, 0xE8, 0xE7, 0xEC, 0xE5, 0xED, 0xE5, 0xED, 0xE8, 0xE9, 0x21, 0x00};
    // You must restart the game for changes to be applied!
    patchBytes("59 6F 75 20 6D 75 73 74 20 72 65 73 74 61 72 74 20 74 68 65 20 67 61 6D 65 20 66 6F 72 20 63 68 61 6E 67 65 73 20 74 6F 20 62 65 20 61 70 70 6C 69 65 64 21 00", restartPromptBytes, sizeof(restartPromptBytes));

    // Хотите ли вы призвать: %s?
    constexpr uint8_t summonBytes[] = {0xD5, 0xEE, 0xF2, 0xE8, 0xF2, 0xE5, 0x20, 0xEB, 0xE8, 0x20, 0xE2, 0xFB, 0x20, 0xEF, 0xF0, 0xE8, 0xE7, 0xE2, 0xE0, 0xF2, 0xFC, 0x3a, 0x20, 0x25, 0x73, 0x3F, 0x00};
    // Would you like to summon %s?
    patchBytes("57 6F 75 6C 64 20 79 6F 75 20 6C 69 6B 65 20 74 6F 20 73 75 6D 6D 6F 6E 20 25 73 3F 00", summonBytes, sizeof(summonBytes));
}
} // namespace TweaksStewie
