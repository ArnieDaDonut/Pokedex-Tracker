#include <stdio.h>
#include <stdint.h>

#define MAPSEC_COUNT 208
#define KANTO_MAPSEC_START 97
#define MAPSEC_POWER_PLANT 151

#define MAPSEC_VIRIDIAN_FOREST 135
#define MAPSEC_MT_MOON 136
#define MAPSEC_DIGLETTS_CAVE 140
#define MAPSEC_KANTO_VICTORY_ROAD 141
#define MAPSEC_POKEMON_MANSION 144
#define MAPSEC_KANTO_SAFARI_ZONE 145
#define MAPSEC_ROCK_TUNNEL 147
#define MAPSEC_SEAFOAM_ISLANDS 148
#define MAPSEC_POKEMON_TOWER 149
#define MAPSEC_CERULEAN_CAVE 150
#define MAPSEC_POWER_PLANT 151

#define MAPSEC_S_S_ANNE 137
#define MAPSEC_UNDERGROUND_PATH 138
#define MAPSEC_UNDERGROUND_PATH_2 139
#define MAPSEC_ROCKET_HIDEOUT 142
#define MAPSEC_SILPH_CO 143
#define MAPSEC_POKEMON_LEAGUE 146

typedef uint8_t u8;
typedef uint16_t u16;

u16 sMapSectionTopLeftCorners[MAPSEC_COUNT][2];

void init_arrays() {
    for (int i = 0; i < MAPSEC_COUNT; i++) {
        sMapSectionTopLeftCorners[i][0] = 0;
        sMapSectionTopLeftCorners[i][1] = 0;
    }
    // Set everything that should have values
    // Assume all non-0,0 have valid values.
    for (int i = 97; i <= 151; i++) {
        // If it's one of the 17, leave it 0,0. Else give it a fake value.
        switch(i) {
            case MAPSEC_VIRIDIAN_FOREST:
            case MAPSEC_MT_MOON:
            case MAPSEC_DIGLETTS_CAVE:
            case MAPSEC_KANTO_VICTORY_ROAD:
            case MAPSEC_POKEMON_MANSION:
            case MAPSEC_KANTO_SAFARI_ZONE:
            case MAPSEC_ROCK_TUNNEL:
            case MAPSEC_SEAFOAM_ISLANDS:
            case MAPSEC_POKEMON_TOWER:
            case MAPSEC_CERULEAN_CAVE:
            case MAPSEC_POWER_PLANT:
            case MAPSEC_S_S_ANNE:
            case MAPSEC_UNDERGROUND_PATH:
            case MAPSEC_UNDERGROUND_PATH_2:
            case MAPSEC_ROCKET_HIDEOUT:
            case MAPSEC_SILPH_CO:
            case MAPSEC_POKEMON_LEAGUE:
                break;
            default:
                sMapSectionTopLeftCorners[i - KANTO_MAPSEC_START][0] = 1;
                sMapSectionTopLeftCorners[i - KANTO_MAPSEC_START][1] = 1;
        }
    }
}

static void GetKantoMapsecCoordinates(u8 mapsec, u8 *mapX, u8 *mapY)
{
    *mapX = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][0];
    *mapY = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][1];

    if (*mapX != 0 || *mapY != 0)
        return;

    switch (mapsec)
    {
        case MAPSEC_VIRIDIAN_FOREST:
            *mapX = 4; *mapY = 6;
            break;
        case MAPSEC_MT_MOON:
            *mapX = 7; *mapY = 3;
            break;
        case MAPSEC_DIGLETTS_CAVE:
            *mapX = 4; *mapY = 7;
            break;
        case MAPSEC_KANTO_VICTORY_ROAD:
            *mapX = 2; *mapY = 5;
            break;
        case MAPSEC_POKEMON_MANSION:
            *mapX = 4; *mapY = 14;
            break;
        case MAPSEC_KANTO_SAFARI_ZONE:
            *mapX = 12; *mapY = 12;
            break;
        case MAPSEC_ROCK_TUNNEL:
            *mapX = 18; *mapY = 4;
            break;
        case MAPSEC_SEAFOAM_ISLANDS:
            *mapX = 9; *mapY = 14;
            break;
        case MAPSEC_POKEMON_TOWER:
            *mapX = 18; *mapY = 6;
            break;
        case MAPSEC_CERULEAN_CAVE:
            *mapX = 14; *mapY = 3;
            break;
        case MAPSEC_POWER_PLANT:
            *mapX = 18; *mapY = 5;
            break;
        case MAPSEC_S_S_ANNE:
            *mapX = 14; *mapY = 9;
            break;
        case MAPSEC_UNDERGROUND_PATH:
            *mapX = 14; *mapY = 5;
            break;
        case MAPSEC_UNDERGROUND_PATH_2:
            *mapX = 15; *mapY = 6;
            break;
        case MAPSEC_ROCKET_HIDEOUT:
            *mapX = 11; *mapY = 6;
            break;
        case MAPSEC_SILPH_CO:
            *mapX = 14; *mapY = 6;
            break;
        case MAPSEC_POKEMON_LEAGUE:
            *mapX = 2; *mapY = 3;
            break;
    }
}

int main() {
    init_arrays();
    for (int i = 97; i <= 151; i++) {
        u8 x = 0, y = 0;
        GetKantoMapsecCoordinates(i, &x, &y);
        if (x == 0 && y == 0) {
            printf("FAILED ON MAPSEC %d\n", i);
        }
    }
    printf("Test complete\n");
    return 0;
}
