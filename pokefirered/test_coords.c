#include <stdio.h>
#include <stdint.h>

#define MAPSEC_COUNT 208
#define KANTO_MAPSEC_START 97
#define MAPSEC_POWER_PLANT 151

typedef uint8_t u8;
typedef uint16_t u16;

static const u16 sMapSectionTopLeftCorners[MAPSEC_COUNT][2] = {
    [97 - 97] = { 4, 11 }, // PALLET_TOWN
    [103 - 97] = { 11, 6 }, // CELADON
    // just dummy
};

int main() {
    printf("Test compiled\n");
    return 0;
}
