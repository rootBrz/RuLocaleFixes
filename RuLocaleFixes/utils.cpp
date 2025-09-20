#include "utils.hpp"
#include "DbgSymbols.hpp"
#include "MinHook.h"
#include <atlconv.h>
#include <string>
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

int FastToLower(int c)
{
    return ((c >= 0x41 && c <= 0x5A) || (c >= 0xC0 && c <= 0xDF)) ? c + 0x20 : c;
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

    switch (key)
    {
    case 'a':
        return 0xF4;
    case 'b':
        return 0xE8;
    case 'c':
        return 0xF1;
    case 'd':
        return 0xE2;
    case 'e':
        return 0xF3;
    case 'f':
        return 0xE0;
    case 'g':
        return 0xEF;
    case 'h':
        return 0xF0;
    case 'i':
        return 0xF8;
    case 'j':
        return 0xEE;
    case 'k':
        return 0xEB;
    case 'l':
        return 0xE4;
    case 'm':
        return 0xFC;
    case 'n':
        return 0xF2;
    case 'o':
        return 0xF9;
    case 'p':
        return 0xE7;
    case 'q':
        return 0xE9;
    case 'r':
        return 0xEA;
    case 's':
        return 0xFB;
    case 't':
        return 0xE5;
    case 'u':
        return 0xE3;
    case 'v':
        return 0xEC;
    case 'w':
        return 0xF6;
    case 'x':
        return 0xF7;
    case 'y':
        return 0xED;
    case 'z':
        return 0xFF;

    case 'A':
        return 0xD4;
    case 'B':
        return 0xC8;
    case 'C':
        return 0xD1;
    case 'D':
        return 0xC2;
    case 'E':
        return 0xD3;
    case 'F':
        return 0xC0;
    case 'G':
        return 0xCF;
    case 'H':
        return 0xD0;
    case 'I':
        return 0xD8;
    case 'J':
        return 0xCE;
    case 'K':
        return 0xCB;
    case 'L':
        return 0xC4;
    case 'M':
        return 0xDC;
    case 'N':
        return 0xD2;
    case 'O':
        return 0xD9;
    case 'P':
        return 0xC7;
    case 'Q':
        return 0xC9;
    case 'R':
        return 0xCA;
    case 'S':
        return 0xDB;
    case 'T':
        return 0xC5;
    case 'U':
        return 0xC3;
    case 'V':
        return 0xCC;
    case 'W':
        return 0xD6;
    case 'X':
        return 0xD7;
    case 'Y':
        return 0xCD;
    case 'Z':
        return 0xDF;

    case ',':
        return 0xE1;
    case '.':
        return 0xFE;
    case '<':
        return 0xC1;
    case '>':
        return 0xDE;
    case ';':
        return 0xE6;
    case ':':
        return 0xC6;
    case '\'':
        return 0xFD;
    case '"':
        return 0xDD;
    case '[':
        return 0xF5;
    case ']':
        return 0xFA;
    case '{':
        return 0xD5;
    case '}':
        return 0xDA;
    }

    return key;
}
