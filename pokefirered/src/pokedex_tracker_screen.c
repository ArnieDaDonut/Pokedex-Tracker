#include "global.h"
#include "gflib.h"
#include "task.h"
#include "menu.h"
#include "list_menu.h"
#include "pokedex.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "region_map.h"
#include "overworld.h"
#include "string_util.h"
#include "strings.h"
#include "malloc.h"
#include "scanline_effect.h"
#include "text_window.h"
#include "window.h"
#include "bg.h"
#include "main.h"
#include "menu_helpers.h"
#include "new_menu_helpers.h"
#include "data.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"
#include "pokedex_tracker_screen.h"

// 386 for National Dex
#define MAX_DEX_MONS 386

struct PokedexTrackerState
{
    u8 state;
    u8 taskId;
    u8 windowId;
    u8 listMenuTaskId;
    u8 mapsecCache[MAX_DEX_MONS + 1];
    struct ListMenuItem listItems[MAX_DEX_MONS];
    u8 itemStrings[MAX_DEX_MONS][32]; // Buffer for the labels
};

static struct PokedexTrackerState *sTracker;

static void CB2_TrackerMain(void);
static void VBlankCB_Tracker(void);
static void Task_TrackerMain(u8 taskId);
static void Tracker_ItemPrintFunc(u8 windowId, u32 itemId, u8 y);

static const u8 sText_Empty[] = _("");
static const u8 sText_Caught[] = _("{COLOR GREEN}CAUGHT");
static const u8 sText_Unknown[] = _("{COLOR RED}UNKNOWN");
static const u8 sText_ColorRed[] = _("{COLOR RED}");

static const struct BgTemplate sTrackerBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    }
};

static const struct WindowTemplate sTrackerWindowTemplate[] =
{
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 28,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 0x001
    },
    DUMMY_WIN_TEMPLATE
};

static const struct ListMenuTemplate sTrackerListMenuTemplate =
{
    .items = NULL,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .itemPrintFunc = Tracker_ItemPrintFunc,
    .totalItems = MAX_DEX_MONS,
    .maxShowed = 8,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_MULTIPLE_SCROLL_DPAD,
    .fontId = 1,
    .cursorKind = 0
};

static void PrecacheEncounters(void)
{
    u32 i;
    for (i = 0; i <= MAX_DEX_MONS; i++)
    {
        sTracker->mapsecCache[i] = MAPSEC_NONE;
    }

    for (i = 0; gWildMonHeaders[i].mapGroup != 0xFF; i++)
    {
        u8 mapsec;
        
        mapsec = Overworld_GetMapHeaderByGroupAndId(gWildMonHeaders[i].mapGroup, gWildMonHeaders[i].mapNum)->regionMapSectionId;
        
        // Land
        if (gWildMonHeaders[i].landMonsInfo)
        {
            u32 j;
            for (j = 0; j < LAND_WILD_COUNT; j++)
            {
                u16 species = gWildMonHeaders[i].landMonsInfo->wildPokemon[j].species;
                u16 natDex = SpeciesToNationalPokedexNum(species);
                if (natDex > 0 && natDex <= MAX_DEX_MONS && sTracker->mapsecCache[natDex] == MAPSEC_NONE)
                    sTracker->mapsecCache[natDex] = mapsec;
            }
        }
        // Water
        if (gWildMonHeaders[i].waterMonsInfo)
        {
            u32 j;
            for (j = 0; j < WATER_WILD_COUNT; j++)
            {
                u16 species = gWildMonHeaders[i].waterMonsInfo->wildPokemon[j].species;
                u16 natDex = SpeciesToNationalPokedexNum(species);
                if (natDex > 0 && natDex <= MAX_DEX_MONS && sTracker->mapsecCache[natDex] == MAPSEC_NONE)
                    sTracker->mapsecCache[natDex] = mapsec;
            }
        }
        // Fishing
        if (gWildMonHeaders[i].fishingMonsInfo)
        {
            u32 j;
            for (j = 0; j < FISH_WILD_COUNT; j++)
            {
                u16 species = gWildMonHeaders[i].fishingMonsInfo->wildPokemon[j].species;
                u16 natDex = SpeciesToNationalPokedexNum(species);
                if (natDex > 0 && natDex <= MAX_DEX_MONS && sTracker->mapsecCache[natDex] == MAPSEC_NONE)
                    sTracker->mapsecCache[natDex] = mapsec;
            }
        }
    }
}

void CB2_InitPokedexTrackerScreen(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ResetVramOamAndBgCntRegs();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = TRUE;
        gMain.state++;
        break;
    case 3:
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 4:
        ResetTasks();
        gMain.state++;
        break;
    case 5:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sTrackerBgTemplates, ARRAY_COUNT(sTrackerBgTemplates));
        gMain.state++;
        break;
    case 6:
        sTracker = AllocZeroed(sizeof(*sTracker));
        PrecacheEncounters();
        gMain.state++;
        break;
    case 7:
        InitWindows(sTrackerWindowTemplate);
        DeactivateAllTextPrinters();
        gMain.state++;
        break;
    case 8:
        LoadStdWindowFrameGfx();
        LoadStdWindowGfx(0, 0x214, BG_PLTT_ID(14));
        gMain.state++;
        break;
    case 9:
        sTracker->windowId = 0;
        DrawStdWindowFrame(sTracker->windowId, FALSE);
        FillWindowPixelBuffer(sTracker->windowId, PIXEL_FILL(1));
        PutWindowTilemap(sTracker->windowId);
        CopyWindowToVram(sTracker->windowId, COPYWIN_FULL);
        gMain.state++;
        break;
    case 10:
    {
        u32 i;
        struct ListMenuTemplate template;
        
        for (i = 0; i < MAX_DEX_MONS; i++)
        {
            u16 natDex = i + 1;
            u16 species = NationalPokedexNumToSpecies(natDex);
            u8 *ptr;
            
            // Format label as "001 Bulbasaur"
            ptr = StringCopy(sTracker->itemStrings[i], sText_Empty);
            ptr = ConvertIntToDecimalStringN(ptr, natDex, STR_CONV_MODE_LEADING_ZEROS, 3);
            *ptr++ = CHAR_SPACE;
            StringCopy(ptr, gSpeciesNames[species]);
            
            sTracker->listItems[i].label = sTracker->itemStrings[i];
            sTracker->listItems[i].index = natDex;
        }

        template = sTrackerListMenuTemplate;
        template.items = sTracker->listItems;
        template.windowId = sTracker->windowId;

        sTracker->listMenuTaskId = ListMenuInit(&template, 0, 0);
        gMain.state++;
        break;
    }
    case 11:
        sTracker->taskId = CreateTask(Task_TrackerMain, 0);
        gMain.state++;
        break;
    case 12:
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gPaletteFade.bufferTransferDisabled = FALSE;
        SetVBlankCallback(VBlankCB_Tracker);
        SetMainCallback2(CB2_TrackerMain);
        break;
    }
}

static void CB2_TrackerMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_Tracker(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Tracker_ItemPrintFunc(u8 windowId, u32 itemId, u8 y)
{
    u16 natDex = itemId;
    u8 text[64];
    u8 *ptr;
    
    if (GetSetPokedexFlag(natDex, FLAG_GET_CAUGHT))
    {
        // Caught
        StringCopy(text, sText_Caught);
    }
    else
    {
        // Uncaught - Show location hint
        if (sTracker->mapsecCache[natDex] != MAPSEC_NONE)
        {
            ptr = StringCopy(text, sText_ColorRed);
            GetMapNameGeneric(ptr, sTracker->mapsecCache[natDex]);
        }
        else
        {
            StringCopy(text, sText_Unknown);
        }
    }
    
    AddTextPrinterParameterized(windowId, 1, text, 120, y, 0xFF, NULL);
}

static void Task_TrackerMain(u8 taskId)
{
    s32 input;
    
    if (gPaletteFade.active)
        return;

    input = ListMenu_ProcessInput(sTracker->listMenuTaskId);
    ListMenuGetScrollAndRow(sTracker->listMenuTaskId, NULL, NULL);

    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        sTracker->state = 1;
    }

    if (sTracker->state == 1 && !gPaletteFade.active)
    {
        DestroyListMenuTask(sTracker->listMenuTaskId, NULL, NULL);
        FreeAllWindowBuffers();
        Free(sTracker);
        SetMainCallback2(CB2_ReturnToField);
        DestroyTask(taskId);
    }
}
