#ifndef GUARD_POKEDEX_TRACKER_H
#define GUARD_POKEDEX_TRACKER_H

#include "global.h"

#define ENCOUNTER_METHOD_NONE       0
#define ENCOUNTER_METHOD_GRASS      1
#define ENCOUNTER_METHOD_SURFING    2
#define ENCOUNTER_METHOD_FISHING    3
#define ENCOUNTER_METHOD_GIFT       4
#define ENCOUNTER_METHOD_TRADE      5
#define ENCOUNTER_METHOD_EVOLUTION  6
#define ENCOUNTER_METHOD_STATIC     7

#define EXCLUSIVE_NONE      0
#define EXCLUSIVE_FIRERED   1
#define EXCLUSIVE_LEAFGREEN 2

struct PokedexTrackerData {
    u16 species;
    const u8 *primaryLocation;
    const u8 *alternateLocations;
    u8 encounterMethod;
    u8 encounterRatePercent;
    u16 requiredItem;
    u8 versionExclusive;
    u16 evolutionFrom;
};

extern const struct PokedexTrackerData gPokedexTrackerData[151];

void ShowPokedexTracker(void);

#endif // GUARD_POKEDEX_TRACKER_H
