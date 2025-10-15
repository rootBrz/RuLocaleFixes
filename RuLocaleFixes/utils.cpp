#include "utils.hpp"
#include "DbgSymbols.hpp"
#include "MinHook.h"
#include <atlconv.h>
#include <string>
#include <unordered_map>
#include <windows.h>

void ResolveAndHook(DbgSymbols &resolver, const char *funcName, void *hook, void **trump)
{
    DWORD64 addr{};
    resolver.GetAddressByName(funcName, addr);
    LPVOID pAddr = reinterpret_cast<LPVOID>(addr);

    if (!pAddr) return;

    MH_CreateHook(pAddr, hook, trump);
    MH_EnableHook(pAddr);
}

int __cdecl FastToUpper(int key)
{
    unsigned char c = (unsigned char)key;
    return (c >= 0x61 && c <= 0x7A) || (c >= 0xE0 && c <= 0xFF) ? c - 0x20 : c;
}

int __cdecl FastToLower(int key)
{
    unsigned char c = (unsigned char)key;
    return (c >= 0x41 && c <= 0x5A) || (c >= 0xC0 && c <= 0xDF) ? c + 0x20 : c;
}

void ConvertStdStringUtf8ToCp1251(std::string &str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len <= 0) return;
    wchar_t *wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, len);

    len = WideCharToMultiByte(1251, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 0) return delete[] wstr, void();
    char *cp1251 = new char[len];
    WideCharToMultiByte(1251, 0, wstr, -1, cp1251, len, NULL, NULL);

    str.assign(cp1251);
    delete[] cp1251;
}

unsigned int TranslateKeyToCP1251(unsigned int key)
{
    HKL layout = GetKeyboardLayout(0);
    LANGID langId = PRIMARYLANGID(LOWORD(layout));

    if (key <= 0x1F || key == 0x7F || langId != LANG_RUSSIAN || GetAsyncKeyState(VK_CONTROL) & 0x8000 || GetAsyncKeyState(VK_MENU) & 0x8000)
        return key;

    std::unordered_map<char, unsigned int> keyMap =
        {
            {'a', 0xF4},
            {'b', 0xE8},
            {'c', 0xF1},
            {'d', 0xE2},
            {'e', 0xF3},
            {'f', 0xE0},
            {'g', 0xEF},
            {'h', 0xF0},
            {'i', 0xF8},
            {'j', 0xEE},
            {'k', 0xEB},
            {'l', 0xE4},
            {'m', 0xFC},
            {'n', 0xF2},
            {'o', 0xF9},
            {'p', 0xE7},
            {'q', 0xE9},
            {'r', 0xEA},
            {'s', 0xFB},
            {'t', 0xE5},
            {'u', 0xE3},
            {'v', 0xEC},
            {'w', 0xF6},
            {'x', 0xF7},
            {'y', 0xED},
            {'z', 0xFF},
            {'A', 0xD4},
            {'B', 0xC8},
            {'C', 0xD1},
            {'D', 0xC2},
            {'E', 0xD3},
            {'F', 0xC0},
            {'G', 0xCF},
            {'H', 0xD0},
            {'I', 0xD8},
            {'J', 0xCE},
            {'K', 0xCB},
            {'L', 0xC4},
            {'M', 0xDC},
            {'N', 0xD2},
            {'O', 0xD9},
            {'P', 0xC7},
            {'Q', 0xC9},
            {'R', 0xCA},
            {'S', 0xDB},
            {'T', 0xC5},
            {'U', 0xC3},
            {'V', 0xCC},
            {'W', 0xD6},
            {'X', 0xD7},
            {'Y', 0xCD},
            {'Z', 0xDF},
            {',', 0xE1},
            {'.', 0xFE},
            {'<', 0xC1},
            {'>', 0xDE},
            {';', 0xE6},
            {':', 0xC6},
            {'\'', 0xFD},
            {'"', 0xDD},
            {'[', 0xF5},
            {']', 0xFA},
            {'{', 0xD5},
            {'}', 0xDA},
            {'@', '"'},
            {'$', ';'},
            {'^', ':'},
            {'&', '?'},
            {'/', '.'},
            {'?', ','},
        };

    auto keyElement = keyMap.find(key);
    return keyElement != keyMap.end() ? keyElement->second : key;
}
