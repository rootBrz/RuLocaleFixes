#include "PatternFinder.hpp"
#include "shared/SafeWrite/SafeWrite.hpp"

namespace TweaksJIP
{
void Apply()
{
    if (!GetModuleHandle("jip_nvse.dll")) return;

    PatternFinder::Scanner search("jip_nvse.dll");

    struct Patch
    {
        const char *pattern;
        const uint8_t *replacement;
        size_t length;
    };

    auto batchPatch = [&search](Patch *patches, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            auto &patch = patches[i];
            for (auto &addr : search.FindPatternAll(patch.pattern))
                if (addr) SafeWriteBuf(addr, const_cast<uint8_t *>(patch.replacement), patch.length);
        }
    };

    // using ё here to match byte length of original, it doesn't display in the game
    constexpr uint8_t atRu[] = {0xE2, 0x20, 0xB8, 0x00}; // "в "
    Patch inAtPatches[] = {
        {"61 74 20 00", atRu, sizeof(atRu)},
        {"69 6E 20 00", atRu, sizeof(atRu)},
    };
    batchPatch(inAtPatches, _countof(inAtPatches));

    // в неизвестной локации
    // at an unknown location
    constexpr uint8_t unknownRu[] = {0xE2, 0x20, 0xED, 0xE5, 0xE8, 0xE7, 0xE2, 0xE5, 0xF1, 0xF2, 0xED, 0xEE, 0xE9, 0x20, 0xEB, 0xEE, 0xEA, 0xE0, 0xF6, 0xE8, 0xE8, 0x00};
    Patch unkPatches[] = {{"61 74 20 61 6E 20 75 6E 6B 6E 6F 77 6E 20 6C 6F 63 61 74 69 6F 6E 00", unknownRu, sizeof(unknownRu)}};
    batchPatch(unkPatches, _countof(unkPatches));

    constexpr uint8_t eastRu[] = {0xC2, 0xEE, 0xF1, 0xF2, 0xEE, 0xEA, 0x20, 0x00}; // "Восток "
    constexpr uint8_t westRu[] = {0xE7, 0xE0, 0xEF, 0xE0, 0xE4, 0x20, 0x00};       // "Запад "
    constexpr uint8_t northRu[] = {0xD1, 0xE5, 0xE2, 0xE5, 0xF0, 0x20, 0x00};      // "Север "
    constexpr uint8_t southRu[] = {0xDE, 0xE3, 0x20, 0x00};                        // "Юг "
    constexpr uint8_t neRu[] = {0xD1, 0xC2, 0x20, 0x00};                           // "СВ "
    constexpr uint8_t seRu[] = {0xDE, 0xC2, 0x20, 0x00};                           // "ЮВ "
    constexpr uint8_t nwRu[] = {0xD1, 0xD7, 0x20, 0x00};                           // "СЗ "
    constexpr uint8_t swRu[] = {0xDE, 0xD7, 0x20, 0x00};                           // "ЮЗ "

    Patch directions[] = {
        {"45 61 73 74 20 6F 66 20 00", eastRu, sizeof(eastRu)},
        {"57 65 73 74 20 6F 66 20 00", westRu, sizeof(westRu)},
        {"4E 6F 72 74 68 20 6F 66 20 00", northRu, sizeof(northRu)},
        {"53 6F 75 74 68 20 6F 66 20 00", southRu, sizeof(southRu)},
        {"4E 45 20 6F 66 20 00", neRu, sizeof(neRu)},
        {"53 45 20 6F 66 20 00", seRu, sizeof(seRu)},
        {"4E 57 20 6F 66 20 00", nwRu, sizeof(nwRu)},
        {"53 57 20 6F 66 20 00", swRu, sizeof(swRu)},
    };
    batchPatch(directions, _countof(directions));

    // The Big Guns skill determines your combat effectiveness with all oversized weapons such as the Fat Man, Missile Launcher, Flamer, Minigun, Gatling Laser, etc.
    // Навык "Тяжелое оружие" определяет эффективность владения крупногабаритным вооружением, таким как "Толстяк", гранатомет, огнемет, миниган, гатлинг-лазер и др.
    constexpr uint8_t bigGunsRu[] = {
        0xCD, 0xE0, 0xE2, 0xFB, 0xEA, 0x20, 0x22, 0xD2, 0xFF, 0xE6, 0xE5, 0xEB,
        0xEE, 0xE5, 0x20, 0xEE, 0xF0, 0xF3, 0xE6, 0xE8, 0xE5, 0x22, 0x20, 0xEE,
        0xEF, 0xF0, 0xE5, 0xE4, 0xE5, 0xEB, 0xFF, 0xE5, 0xF2, 0x20, 0xFD, 0xF4,
        0xF4, 0xE5, 0xEA, 0xF2, 0xE8, 0xE2, 0xED, 0xEE, 0xF1, 0xF2, 0xFC, 0x20,
        0xE2, 0xEB, 0xE0, 0xE4, 0xE5, 0xED, 0xE8, 0xFF, 0x20, 0xEA, 0xF0, 0xF3,
        0xEF, 0xED, 0xEE, 0xE3, 0xE0, 0xE1, 0xE0, 0xF0, 0xE8, 0xF2, 0xED, 0xFB,
        0xEC, 0x20, 0xE2, 0xEE, 0xEE, 0xF0, 0xF3, 0xE6, 0xE5, 0xED, 0xE8, 0xE5,
        0xEC, 0x2C, 0x20, 0xF2, 0xE0, 0xEA, 0xE8, 0xEC, 0x20, 0xEA, 0xE0, 0xEA,
        0x20, 0x22, 0xD2, 0xEE, 0xEB, 0xF1, 0xF2, 0xFF, 0xEA, 0x22, 0x2C,
        0x20, 0xE3, 0xF0, 0xE0, 0xED, 0xE0, 0xF2, 0xEE, 0xEC, 0xE5, 0xF2, 0x2C,
        0x20, 0xEE, 0xE3, 0xED, 0xE5, 0xEC, 0xE5, 0xF2, 0x2C, 0x20, 0xEC, 0xE8,
        0xED, 0xE8, 0xE3, 0xE0, 0xED, 0x2C, 0x20, 0xE3, 0xE0, 0xF2, 0xEB, 0xE8,
        0xED, 0xE3, 0x2D, 0xEB, 0xE0, 0xE7, 0xE5, 0xF0, 0x20, 0xE8, 0x20, 0xE4,
        0xF0, 0x2E, 0x00};
    Patch bigGunsPatches[] = {{"54 68 65 20 42 69 67 20 47 75 6E 73 20 73 6B 69 "
                               "6C 6C 20 64 65 74 65 72 "
                               "6D 69 6E 65 73 20 79 6F 75 72 20 63 6F 6D 62 61 "
                               "74 20 65 66 66 65 63 74 "
                               "69 76 65 6E 65 73 73 20 77 69 74 68 20 61 6C 6C "
                               "20 6F 76 65 72 73 69 7A "
                               "65 64 20 77 65 61 70 6F 6E 73 20 73 75 63 68 20 "
                               "61 73 20 74 68 65 20 46 "
                               "61 74 20 4D 61 6E 2C 20 4D 69 73 73 69 6C 65 20 "
                               "4C 61 75 6E 63 68 65 72 "
                               "2C 20 46 6C 61 6D 65 72 2C 20 4D 69 6E 69 67 75 "
                               "6E 2C 20 47 61 74 6C 69 "
                               "6E 67 20 4C 61 73 65 72 2C 20 65 74 63 2E 00",
                               bigGunsRu, sizeof(bigGunsRu)}};
    batchPatch(bigGunsPatches, _countof(bigGunsPatches));
}
} // namespace TweaksJIP
