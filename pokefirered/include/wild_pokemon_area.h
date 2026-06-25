#ifndef GUARD_WILD_POKEMON_AREA_H
#define GUARD_WILD_POKEMON_AREA_H

#include "wild_encounter.h"

s32 GetSpeciesPokedexAreaMarkers(u16 species, struct Subsprite * subsprites);
bool32 IsSpeciesOnMap(const struct WildPokemonHeader * data, s32 species);
u16 GetMapSecIdFromWildMonHeader(const struct WildPokemonHeader * header);

#endif //GUARD_WILD_POKEMON_AREA_H
