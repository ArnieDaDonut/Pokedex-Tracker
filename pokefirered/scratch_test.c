#include "global.h"
// pseudocode for getting mapsecs
void CreatePokemonMapIcons(u16 species) {
    u32 i;
    u8 drawnMapsecs[100] = {0};
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++) {
        if (IsSpeciesOnMap(&gWildMonHeaders[i], species)) {
            u8 mapsec = GetMapSecIdFromWildMonHeader(&gWildMonHeaders[i]);
            if (!drawnMapsecs[mapsec]) {
                drawnMapsecs[mapsec] = 1;
                // get X, Y
                u8 mapX = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][0];
                u8 mapY = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][1];
                
                // CreateMonIcon
                CreateMonIcon(species, SpriteCallbackDummy, (mapX * 8) + 8, (mapY * 8) + 8, 0, 0xFFFFFFFF, FALSE);
            }
        }
    }
}
