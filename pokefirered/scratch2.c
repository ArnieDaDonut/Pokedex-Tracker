static u8 sTrackerIconSpriteIds[32];
static u8 sTrackerIconSpriteCount = 0;

static void CreatePokemonTrackerIcons(void)
{
    u32 i;
    u8 drawnMapsecs[MAPSEC_COUNT] = {0};

    sTrackerIconSpriteCount = 0;
    if (gTrackerMapSpecies == SPECIES_NONE)
        return;

    LoadMonIconPalettes();

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (IsSpeciesOnMap(&gWildMonHeaders[i], gTrackerMapSpecies))
        {
            u8 mapsec = GetMapSecIdFromWildMonHeader(&gWildMonHeaders[i]);
            if (mapsec >= KANTO_MAPSEC_START && mapsec < MAPSEC_COUNT && !drawnMapsecs[mapsec])
            {
                drawnMapsecs[mapsec] = 1;
                if (sTrackerIconSpriteCount < 32)
                {
                    u8 mapX = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][0];
                    u8 mapY = sMapSectionTopLeftCorners[mapsec - KANTO_MAPSEC_START][1];
                    u16 x_pixel = 8 * mapX + 36;
                    u16 y_pixel = 8 * mapY + 28;
                    sTrackerIconSpriteIds[sTrackerIconSpriteCount++] = CreateMonIcon(gTrackerMapSpecies, SpriteCallbackDummy, x_pixel, y_pixel, 0, 0xFFFFFFFF, FALSE);
                }
            }
        }
    }
}

static void FreePokemonTrackerIcons(void)
{
    u8 i;
    for (i = 0; i < sTrackerIconSpriteCount; i++)
    {
        FreeAndDestroyMonIconSprite(&gSprites[sTrackerIconSpriteIds[i]]);
    }
    if (sTrackerIconSpriteCount > 0)
        FreeMonIconPalettes();
    sTrackerIconSpriteCount = 0;
}
