#include "global.h"
#include "constants/species.h"
#include "constants/items.h"
#include "pokedex_tracker.h"

// Example subset of data for the first 9 Pokemon.
// The rest of the 151 would follow this exact format.
const struct PokedexTrackerData gPokedexTrackerData[151] = {
    [SPECIES_BULBASAUR - 1] = {
        .species = SPECIES_BULBASAUR,
        .primaryLocation = _("Pallet Town"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_GIFT,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_NONE
    },
    [SPECIES_IVYSAUR - 1] = {
        .species = SPECIES_IVYSAUR,
        .primaryLocation = _("Evolve Bulbasaur"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_BULBASAUR
    },
    [SPECIES_VENUSAUR - 1] = {
        .species = SPECIES_VENUSAUR,
        .primaryLocation = _("Evolve Ivysaur"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_IVYSAUR
    },
    [SPECIES_CHARMANDER - 1] = {
        .species = SPECIES_CHARMANDER,
        .primaryLocation = _("Pallet Town"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_GIFT,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_NONE
    },
    [SPECIES_CHARMELEON - 1] = {
        .species = SPECIES_CHARMELEON,
        .primaryLocation = _("Evolve Charmander"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_CHARMANDER
    },
    [SPECIES_CHARIZARD - 1] = {
        .species = SPECIES_CHARIZARD,
        .primaryLocation = _("Evolve Charmeleon"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_CHARMELEON
    },
    [SPECIES_SQUIRTLE - 1] = {
        .species = SPECIES_SQUIRTLE,
        .primaryLocation = _("Pallet Town"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_GIFT,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_NONE
    },
    [SPECIES_WARTORTLE - 1] = {
        .species = SPECIES_WARTORTLE,
        .primaryLocation = _("Evolve Squirtle"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_SQUIRTLE
    },
    [SPECIES_BLASTOISE - 1] = {
        .species = SPECIES_BLASTOISE,
        .primaryLocation = _("Evolve Wartortle"),
        .alternateLocations = _("None"),
        .encounterMethod = ENCOUNTER_METHOD_EVOLUTION,
        .encounterRatePercent = 100,
        .requiredItem = ITEM_NONE,
        .versionExclusive = EXCLUSIVE_NONE,
        .evolutionFrom = SPECIES_WARTORTLE
    },
    // The rest of the 151 Pokemon go here...
};

// UI Task States
enum {
    STATE_INIT,
    STATE_HOME_SCREEN,
    STATE_FULL_DEX_SCREEN,
    STATE_MISSING_DEX_SCREEN,
    STATE_DETAIL_SCREEN,
    STATE_EXIT,
};

static void Task_PokedexTracker_Main(u8 taskId);
static void CB2_PokedexTracker_Main(void);
static void VBlankCB_PokedexTracker(void);

// UI and Logic hooks
void ShowPokedexTracker(void) {
    // Basic setup for a new screen
    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
    
    // Disable interrupts while setting up
    DisableInterrupts(INTR_FLAG_VBLANK);
    
    // Clear graphics state
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    
    // Re-enable interrupts
    EnableInterrupts(INTR_FLAG_VBLANK);
    
    // Create task for our UI state machine
    CreateTask(Task_PokedexTracker_Main, 0);
    SetMainCallback2(CB2_PokedexTracker_Main);
}

static void CB2_PokedexTracker_Main(void) {
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_PokedexTracker(void) {
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void Task_PokedexTracker_Main(u8 taskId) {
    s16 *data = gTasks[taskId].data;
    
    switch (data[0]) {
        case STATE_INIT:
            // TODO: Load Tiles, Palettes, and Tilemaps here.
            // Setup Window and Text configurations.
            SetVBlankCallback(VBlankCB_PokedexTracker);
            SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG0_ON);
            data[0] = STATE_HOME_SCREEN;
            break;
            
        case STATE_HOME_SCREEN:
            // TODO: Draw progress bar and stats
            // Handle D-PAD to switch modes or B to exit
            if (JOY_NEW(B_BUTTON)) {
                data[0] = STATE_EXIT;
            } else if (JOY_NEW(A_BUTTON)) {
                data[0] = STATE_FULL_DEX_SCREEN;
            }
            break;
            
        case STATE_FULL_DEX_SCREEN:
            // TODO: Render scrollable 151 list
            if (JOY_NEW(B_BUTTON)) {
                data[0] = STATE_HOME_SCREEN;
            } else if (JOY_NEW(SELECT_BUTTON)) {
                data[0] = STATE_MISSING_DEX_SCREEN;
            } else if (JOY_NEW(A_BUTTON)) {
                // Assuming we selected a pokemon
                data[0] = STATE_DETAIL_SCREEN;
            }
            break;
            
        case STATE_MISSING_DEX_SCREEN:
            // TODO: Render filtered uncaught list
            if (JOY_NEW(B_BUTTON)) {
                data[0] = STATE_HOME_SCREEN;
            } else if (JOY_NEW(SELECT_BUTTON)) {
                data[0] = STATE_FULL_DEX_SCREEN;
            }
            break;
            
        case STATE_DETAIL_SCREEN:
            // TODO: Render single Pokemon + location info
            // Fetch caught flag example: s8 isCaught = GetSetPokedexFlag(nationalNum, FLAG_GET_CAUGHT);
            if (JOY_NEW(B_BUTTON)) {
                data[0] = STATE_FULL_DEX_SCREEN;
            }
            break;
            
        case STATE_EXIT:
            // Cleanup and return to overworld
            SetMainCallback2(CB2_ReturnToField);
            DestroyTask(taskId);
            break;
    }
}

