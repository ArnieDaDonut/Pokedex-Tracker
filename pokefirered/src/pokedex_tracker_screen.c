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
#include "pokemon_icon.h"
#include "naming_screen.h"
#include "sound.h"
#include "constants/songs.h"

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
    u8 monIconSpriteIds[8];
    u32 totalItems;
    u8 searchQuery[12];
    bool8 searchFocus;
    u8 cursorCol;
};

static struct PokedexTrackerState *sTracker;

static void CB2_TrackerMain(void);
static void VBlankCB_Tracker(void);
static void Task_TrackerMain(u8 taskId);
static void Tracker_ItemPrintFunc(u8 windowId, u32 itemId, u8 y);
static void Tracker_CursorMoveFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static void CleanupTracker(u8 taskId);
static bool8 StringStartsWith(const u8 *str, const u8 *prefix);

static const u8 sText_Empty[] = _("");
static const u8 sText_Caught[] = _("{COLOR GREEN}CAUGHT");
static const u8 sText_Unknown[] = _("{COLOR RED}UNKNOWN");
static const u8 sText_ColorRed[] = _("{COLOR RED}");
static const u8 sText_Cursor[] = _("{RIGHT_ARROW}");
static const u8 sText_Search[] = _("SEARCH: ");
static const u8 sText_StatusColumn[] = _("STATUS");
static const u8 sText_MarkCaught[] = _("{COLOR GREEN}O");
static const u8 sText_MarkNotCaught[] = _("{COLOR RED}X");
static const u8 sText_LocationBtn[] = _("{COLOR WHITE}{SHADOW LIGHT_GRAY}{HIGHLIGHT BLUE} LOCATION ");

static const struct BgTemplate sTrackerBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
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
        .paletteNum = 14,  // Matches LoadStdWindowGfx
        .baseBlock = 0x001
    },
    DUMMY_WIN_TEMPLATE
};

static const struct ListMenuTemplate sTrackerListMenuTemplate =
{
    .items = NULL,
    .moveCursorFunc = Tracker_CursorMoveFunc,
    .itemPrintFunc = Tracker_ItemPrintFunc,
    .totalItems = MAX_DEX_MONS,
    .maxShowed = 4,
    .windowId = 0,
    .header_X = 0,
    .item_X = 40,
    .cursor_X = 0,
    .upText_Y = 4,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_MULTIPLE_SCROLL_L_R,
    .fontId = 1,
    .cursorKind = 1
};

static const u8 sLoc_0[] = _("?");
static const u8 sLoc_1[] = _("PALLET TOWN");
static const u8 sLoc_2[] = _("EVOLVE");
static const u8 sLoc_3[] = _("EVOLVE");
static const u8 sLoc_4[] = _("PALLET TOWN");
static const u8 sLoc_5[] = _("EVOLVE");
static const u8 sLoc_6[] = _("EVOLVE");
static const u8 sLoc_7[] = _("PALLET TOWN");
static const u8 sLoc_8[] = _("EVOLVE");
static const u8 sLoc_9[] = _("EVOLVE");
static const u8 sLoc_10[] = _("VIRIDIAN FORES");
static const u8 sLoc_11[] = _("VIRIDIAN FORES");
static const u8 sLoc_12[] = _("EVOLVE");
static const u8 sLoc_13[] = _("VIRIDIAN FORES");
static const u8 sLoc_14[] = _("VIRIDIAN FORES");
static const u8 sLoc_15[] = _("EVOLVE");
static const u8 sLoc_16[] = _("ROUTES 1, 2, 3");
static const u8 sLoc_17[] = _("ROUTES 13, 14,");
static const u8 sLoc_18[] = _("EVOLVE");
static const u8 sLoc_19[] = _("ROUTES 1, 2, 4");
static const u8 sLoc_20[] = _("ROUTES 16, 17,");
static const u8 sLoc_21[] = _("ROUTES 3, 4, 9");
static const u8 sLoc_22[] = _("ROUTES 17, 18,");
static const u8 sLoc_23[] = _("ROUTES 4, 8, 9");
static const u8 sLoc_24[] = _("ROUTE 23");
static const u8 sLoc_25[] = _("POWER PLANT");
static const u8 sLoc_26[] = _("EVOLVE");
static const u8 sLoc_27[] = _("ROUTES 4, 8, 9");
static const u8 sLoc_28[] = _("ROUTE 23");
static const u8 sLoc_29[] = _("POISON");
static const u8 sLoc_30[] = _("POISON");
static const u8 sLoc_31[] = _("EVOLVE");
static const u8 sLoc_32[] = _("POISON");
static const u8 sLoc_33[] = _("POISON");
static const u8 sLoc_34[] = _("EVOLVE");
static const u8 sLoc_35[] = _("NORMAL");
static const u8 sLoc_36[] = _("EVOLVE");
static const u8 sLoc_37[] = _("ROUTES 7 AND 8");
static const u8 sLoc_38[] = _("EVOLVE");
static const u8 sLoc_39[] = _("ROUTE 3");
static const u8 sLoc_40[] = _("EVOLVE");
static const u8 sLoc_41[] = _("MT. MOON");
static const u8 sLoc_42[] = _("EVOLVE");
static const u8 sLoc_43[] = _("ROUTES 5, 6, 7");
static const u8 sLoc_44[] = _("ROUTES 12, 13,");
static const u8 sLoc_45[] = _("EVOLVE");
static const u8 sLoc_46[] = _("SAFARI ZONE");
static const u8 sLoc_47[] = _("SAFARI ZONE");
static const u8 sLoc_48[] = _("ROUTES 12, 13,");
static const u8 sLoc_49[] = _("SAFARI ZONE");
static const u8 sLoc_50[] = _("DIGLETT CAVE");
static const u8 sLoc_51[] = _("EVOLVE");
static const u8 sLoc_52[] = _("ROUTES 5, 6, 7");
static const u8 sLoc_53[] = _("EVOLVE");
static const u8 sLoc_54[] = _("PALLET TOWN");
static const u8 sLoc_55[] = _("EVOLVE");
static const u8 sLoc_56[] = _("ROUTES 3, 4, 2");
static const u8 sLoc_57[] = _("ROUTE 23");
static const u8 sLoc_58[] = _("ROUTES 7 AND 8");
static const u8 sLoc_59[] = _("EVOLVE");
static const u8 sLoc_60[] = _("FUCHSIA CITY, ");
static const u8 sLoc_61[] = _("ROUTES 6, 22, ");
static const u8 sLoc_62[] = _("EVOLVE");
static const u8 sLoc_63[] = _("PSYCHIC");
static const u8 sLoc_64[] = _("EVOLVE");
static const u8 sLoc_65[] = _("EVOLVE");
static const u8 sLoc_66[] = _("VICTORY ROAD");
static const u8 sLoc_67[] = _("EVOLVE");
static const u8 sLoc_68[] = _("EVOLVE");
static const u8 sLoc_69[] = _("ROUTES 5, 6, 7");
static const u8 sLoc_70[] = _("ROUTES 12, 13,");
static const u8 sLoc_71[] = _("EVOLVE");
static const u8 sLoc_72[] = _("PALLET TOWN");
static const u8 sLoc_73[] = _("EVOLVE");
static const u8 sLoc_74[] = _("MT. MOON");
static const u8 sLoc_75[] = _("EVOLVE");
static const u8 sLoc_76[] = _("EVOLVE");
static const u8 sLoc_77[] = _("KINDLE ROAD AN");
static const u8 sLoc_78[] = _("EVOLVE");
static const u8 sLoc_79[] = _("PALLET TOWN");
static const u8 sLoc_80[] = _("EVOLVE");
static const u8 sLoc_81[] = _("POWER PLANT");
static const u8 sLoc_82[] = _("EVOLVE");
static const u8 sLoc_83[] = _("TRADE");
static const u8 sLoc_84[] = _("ROUTES 16, 17,");
static const u8 sLoc_85[] = _("EVOLVE");
static const u8 sLoc_86[] = _("TRADE");
static const u8 sLoc_87[] = _("EVOLVE");
static const u8 sLoc_88[] = _("CELADON CITY");
static const u8 sLoc_89[] = _("POISON");
static const u8 sLoc_90[] = _("PALLET TOWN");
static const u8 sLoc_91[] = _("EVOLVE");
static const u8 sLoc_92[] = _("PKMN TOWER");
static const u8 sLoc_93[] = _("EVOLVE");
static const u8 sLoc_94[] = _("EVOLVE");
static const u8 sLoc_95[] = _("VICTORY ROAD");
static const u8 sLoc_96[] = _("ROUTE 11");
static const u8 sLoc_97[] = _("EVOLVE");
static const u8 sLoc_98[] = _("PALLET TOWN");
static const u8 sLoc_99[] = _("PALLET TOWN");
static const u8 sLoc_100[] = _("ROUTE 10");
static const u8 sLoc_101[] = _("EVOLVE");
static const u8 sLoc_102[] = _("SAFARI ZONE");
static const u8 sLoc_103[] = _("EVOLVE");
static const u8 sLoc_104[] = _("PKMN TOWER");
static const u8 sLoc_105[] = _("EVOLVE");
static const u8 sLoc_106[] = _("SAFFRON CITY");
static const u8 sLoc_107[] = _("SAFFRON CITY");
static const u8 sLoc_108[] = _("NORMAL");
static const u8 sLoc_109[] = _("CELADON CITY");
static const u8 sLoc_110[] = _("POISON");
static const u8 sLoc_111[] = _("SAFARI ZONE");
static const u8 sLoc_112[] = _("EVOLVE");
static const u8 sLoc_113[] = _("SAFARI ZONE");
static const u8 sLoc_114[] = _("ROUTE 21");
static const u8 sLoc_115[] = _("SAFARI ZONE");
static const u8 sLoc_116[] = _("WATER");
static const u8 sLoc_117[] = _("WATER");
static const u8 sLoc_118[] = _("ROUTES 6, 22, ");
static const u8 sLoc_119[] = _("SAFARI ZONE");
static const u8 sLoc_120[] = _("PALLET TOWN");
static const u8 sLoc_121[] = _("EVOLVE");
static const u8 sLoc_122[] = _("ROUTE 2");
static const u8 sLoc_123[] = _("SAFARI ZONE");
static const u8 sLoc_124[] = _("TRADE");
static const u8 sLoc_125[] = _("POWER PLANT");
static const u8 sLoc_126[] = _("MT. EMBER");
static const u8 sLoc_127[] = _("SAFARI ZONE");
static const u8 sLoc_128[] = _("SAFARI ZONE");
static const u8 sLoc_129[] = _("PALLET TOWN");
static const u8 sLoc_130[] = _("PALLET TOWN");
static const u8 sLoc_131[] = _("SILPH CO.");
static const u8 sLoc_132[] = _("ROUTES 13, 14,");
static const u8 sLoc_133[] = _("CELADON CITY");
static const u8 sLoc_134[] = _("EVOLVE");
static const u8 sLoc_135[] = _("EVOLVE");
static const u8 sLoc_136[] = _("EVOLVE");
static const u8 sLoc_137[] = _("NORMAL");
static const u8 sLoc_138[] = _("MT. MOON");
static const u8 sLoc_139[] = _("EVOLVE");
static const u8 sLoc_140[] = _("MT. MOON");
static const u8 sLoc_141[] = _("EVOLVE");
static const u8 sLoc_142[] = _("CINNABAR ISL.");
static const u8 sLoc_143[] = _("ROUTES 12 AND ");
static const u8 sLoc_144[] = _("SEAFOAM IS.");
static const u8 sLoc_145[] = _("POWER PLANT");
static const u8 sLoc_146[] = _("MT. EMBER");
static const u8 sLoc_147[] = _("DRAGON");
static const u8 sLoc_148[] = _("SAFARI ZONE");
static const u8 sLoc_149[] = _("EVOLVE");
static const u8 sLoc_150[] = _("CERULEAN CAVE");
static const u8 sLoc_151[] = _("EVENT");
static const u8 sLoc_152[] = _("UNKNOWN");
static const u8 sLoc_153[] = _("UNKNOWN");
static const u8 sLoc_154[] = _("UNKNOWN");
static const u8 sLoc_155[] = _("UNKNOWN");
static const u8 sLoc_156[] = _("UNKNOWN");
static const u8 sLoc_157[] = _("UNKNOWN");
static const u8 sLoc_158[] = _("UNKNOWN");
static const u8 sLoc_159[] = _("UNKNOWN");
static const u8 sLoc_160[] = _("UNKNOWN");
static const u8 sLoc_161[] = _("UNKNOWN");
static const u8 sLoc_162[] = _("UNKNOWN");
static const u8 sLoc_163[] = _("UNKNOWN");
static const u8 sLoc_164[] = _("UNKNOWN");
static const u8 sLoc_165[] = _("UNKNOWN");
static const u8 sLoc_166[] = _("UNKNOWN");
static const u8 sLoc_167[] = _("UNKNOWN");
static const u8 sLoc_168[] = _("UNKNOWN");
static const u8 sLoc_169[] = _("UNKNOWN");
static const u8 sLoc_170[] = _("UNKNOWN");
static const u8 sLoc_171[] = _("UNKNOWN");
static const u8 sLoc_172[] = _("UNKNOWN");
static const u8 sLoc_173[] = _("UNKNOWN");
static const u8 sLoc_174[] = _("UNKNOWN");
static const u8 sLoc_175[] = _("UNKNOWN");
static const u8 sLoc_176[] = _("UNKNOWN");
static const u8 sLoc_177[] = _("UNKNOWN");
static const u8 sLoc_178[] = _("UNKNOWN");
static const u8 sLoc_179[] = _("UNKNOWN");
static const u8 sLoc_180[] = _("UNKNOWN");
static const u8 sLoc_181[] = _("UNKNOWN");
static const u8 sLoc_182[] = _("UNKNOWN");
static const u8 sLoc_183[] = _("UNKNOWN");
static const u8 sLoc_184[] = _("UNKNOWN");
static const u8 sLoc_185[] = _("UNKNOWN");
static const u8 sLoc_186[] = _("UNKNOWN");
static const u8 sLoc_187[] = _("UNKNOWN");
static const u8 sLoc_188[] = _("UNKNOWN");
static const u8 sLoc_189[] = _("UNKNOWN");
static const u8 sLoc_190[] = _("UNKNOWN");
static const u8 sLoc_191[] = _("UNKNOWN");
static const u8 sLoc_192[] = _("UNKNOWN");
static const u8 sLoc_193[] = _("UNKNOWN");
static const u8 sLoc_194[] = _("UNKNOWN");
static const u8 sLoc_195[] = _("UNKNOWN");
static const u8 sLoc_196[] = _("UNKNOWN");
static const u8 sLoc_197[] = _("UNKNOWN");
static const u8 sLoc_198[] = _("UNKNOWN");
static const u8 sLoc_199[] = _("UNKNOWN");
static const u8 sLoc_200[] = _("UNKNOWN");
static const u8 sLoc_201[] = _("UNKNOWN");
static const u8 sLoc_202[] = _("UNKNOWN");
static const u8 sLoc_203[] = _("UNKNOWN");
static const u8 sLoc_204[] = _("UNKNOWN");
static const u8 sLoc_205[] = _("UNKNOWN");
static const u8 sLoc_206[] = _("UNKNOWN");
static const u8 sLoc_207[] = _("UNKNOWN");
static const u8 sLoc_208[] = _("UNKNOWN");
static const u8 sLoc_209[] = _("UNKNOWN");
static const u8 sLoc_210[] = _("UNKNOWN");
static const u8 sLoc_211[] = _("UNKNOWN");
static const u8 sLoc_212[] = _("UNKNOWN");
static const u8 sLoc_213[] = _("UNKNOWN");
static const u8 sLoc_214[] = _("UNKNOWN");
static const u8 sLoc_215[] = _("UNKNOWN");
static const u8 sLoc_216[] = _("UNKNOWN");
static const u8 sLoc_217[] = _("UNKNOWN");
static const u8 sLoc_218[] = _("UNKNOWN");
static const u8 sLoc_219[] = _("UNKNOWN");
static const u8 sLoc_220[] = _("UNKNOWN");
static const u8 sLoc_221[] = _("UNKNOWN");
static const u8 sLoc_222[] = _("UNKNOWN");
static const u8 sLoc_223[] = _("UNKNOWN");
static const u8 sLoc_224[] = _("UNKNOWN");
static const u8 sLoc_225[] = _("UNKNOWN");
static const u8 sLoc_226[] = _("UNKNOWN");
static const u8 sLoc_227[] = _("UNKNOWN");
static const u8 sLoc_228[] = _("UNKNOWN");
static const u8 sLoc_229[] = _("UNKNOWN");
static const u8 sLoc_230[] = _("UNKNOWN");
static const u8 sLoc_231[] = _("UNKNOWN");
static const u8 sLoc_232[] = _("UNKNOWN");
static const u8 sLoc_233[] = _("UNKNOWN");
static const u8 sLoc_234[] = _("UNKNOWN");
static const u8 sLoc_235[] = _("UNKNOWN");
static const u8 sLoc_236[] = _("UNKNOWN");
static const u8 sLoc_237[] = _("UNKNOWN");
static const u8 sLoc_238[] = _("UNKNOWN");
static const u8 sLoc_239[] = _("UNKNOWN");
static const u8 sLoc_240[] = _("UNKNOWN");
static const u8 sLoc_241[] = _("UNKNOWN");
static const u8 sLoc_242[] = _("UNKNOWN");
static const u8 sLoc_243[] = _("UNKNOWN");
static const u8 sLoc_244[] = _("UNKNOWN");
static const u8 sLoc_245[] = _("UNKNOWN");
static const u8 sLoc_246[] = _("UNKNOWN");
static const u8 sLoc_247[] = _("UNKNOWN");
static const u8 sLoc_248[] = _("UNKNOWN");
static const u8 sLoc_249[] = _("UNKNOWN");
static const u8 sLoc_250[] = _("UNKNOWN");
static const u8 sLoc_251[] = _("UNKNOWN");
static const u8 sLoc_252[] = _("UNKNOWN");
static const u8 sLoc_253[] = _("UNKNOWN");
static const u8 sLoc_254[] = _("UNKNOWN");
static const u8 sLoc_255[] = _("UNKNOWN");
static const u8 sLoc_256[] = _("UNKNOWN");
static const u8 sLoc_257[] = _("UNKNOWN");
static const u8 sLoc_258[] = _("UNKNOWN");
static const u8 sLoc_259[] = _("UNKNOWN");
static const u8 sLoc_260[] = _("UNKNOWN");
static const u8 sLoc_261[] = _("UNKNOWN");
static const u8 sLoc_262[] = _("UNKNOWN");
static const u8 sLoc_263[] = _("UNKNOWN");
static const u8 sLoc_264[] = _("UNKNOWN");
static const u8 sLoc_265[] = _("UNKNOWN");
static const u8 sLoc_266[] = _("UNKNOWN");
static const u8 sLoc_267[] = _("UNKNOWN");
static const u8 sLoc_268[] = _("UNKNOWN");
static const u8 sLoc_269[] = _("UNKNOWN");
static const u8 sLoc_270[] = _("UNKNOWN");
static const u8 sLoc_271[] = _("UNKNOWN");
static const u8 sLoc_272[] = _("UNKNOWN");
static const u8 sLoc_273[] = _("UNKNOWN");
static const u8 sLoc_274[] = _("UNKNOWN");
static const u8 sLoc_275[] = _("UNKNOWN");
static const u8 sLoc_276[] = _("UNKNOWN");
static const u8 sLoc_277[] = _("UNKNOWN");
static const u8 sLoc_278[] = _("UNKNOWN");
static const u8 sLoc_279[] = _("UNKNOWN");
static const u8 sLoc_280[] = _("UNKNOWN");
static const u8 sLoc_281[] = _("UNKNOWN");
static const u8 sLoc_282[] = _("UNKNOWN");
static const u8 sLoc_283[] = _("UNKNOWN");
static const u8 sLoc_284[] = _("UNKNOWN");
static const u8 sLoc_285[] = _("UNKNOWN");
static const u8 sLoc_286[] = _("UNKNOWN");
static const u8 sLoc_287[] = _("UNKNOWN");
static const u8 sLoc_288[] = _("UNKNOWN");
static const u8 sLoc_289[] = _("UNKNOWN");
static const u8 sLoc_290[] = _("UNKNOWN");
static const u8 sLoc_291[] = _("UNKNOWN");
static const u8 sLoc_292[] = _("UNKNOWN");
static const u8 sLoc_293[] = _("UNKNOWN");
static const u8 sLoc_294[] = _("UNKNOWN");
static const u8 sLoc_295[] = _("UNKNOWN");
static const u8 sLoc_296[] = _("UNKNOWN");
static const u8 sLoc_297[] = _("UNKNOWN");
static const u8 sLoc_298[] = _("UNKNOWN");
static const u8 sLoc_299[] = _("UNKNOWN");
static const u8 sLoc_300[] = _("UNKNOWN");
static const u8 sLoc_301[] = _("UNKNOWN");
static const u8 sLoc_302[] = _("UNKNOWN");
static const u8 sLoc_303[] = _("UNKNOWN");
static const u8 sLoc_304[] = _("UNKNOWN");
static const u8 sLoc_305[] = _("UNKNOWN");
static const u8 sLoc_306[] = _("UNKNOWN");
static const u8 sLoc_307[] = _("UNKNOWN");
static const u8 sLoc_308[] = _("UNKNOWN");
static const u8 sLoc_309[] = _("UNKNOWN");
static const u8 sLoc_310[] = _("UNKNOWN");
static const u8 sLoc_311[] = _("UNKNOWN");
static const u8 sLoc_312[] = _("UNKNOWN");
static const u8 sLoc_313[] = _("UNKNOWN");
static const u8 sLoc_314[] = _("UNKNOWN");
static const u8 sLoc_315[] = _("UNKNOWN");
static const u8 sLoc_316[] = _("UNKNOWN");
static const u8 sLoc_317[] = _("UNKNOWN");
static const u8 sLoc_318[] = _("UNKNOWN");
static const u8 sLoc_319[] = _("UNKNOWN");
static const u8 sLoc_320[] = _("UNKNOWN");
static const u8 sLoc_321[] = _("UNKNOWN");
static const u8 sLoc_322[] = _("UNKNOWN");
static const u8 sLoc_323[] = _("UNKNOWN");
static const u8 sLoc_324[] = _("UNKNOWN");
static const u8 sLoc_325[] = _("UNKNOWN");
static const u8 sLoc_326[] = _("UNKNOWN");
static const u8 sLoc_327[] = _("UNKNOWN");
static const u8 sLoc_328[] = _("UNKNOWN");
static const u8 sLoc_329[] = _("UNKNOWN");
static const u8 sLoc_330[] = _("UNKNOWN");
static const u8 sLoc_331[] = _("UNKNOWN");
static const u8 sLoc_332[] = _("UNKNOWN");
static const u8 sLoc_333[] = _("UNKNOWN");
static const u8 sLoc_334[] = _("UNKNOWN");
static const u8 sLoc_335[] = _("UNKNOWN");
static const u8 sLoc_336[] = _("UNKNOWN");
static const u8 sLoc_337[] = _("UNKNOWN");
static const u8 sLoc_338[] = _("UNKNOWN");
static const u8 sLoc_339[] = _("UNKNOWN");
static const u8 sLoc_340[] = _("UNKNOWN");
static const u8 sLoc_341[] = _("UNKNOWN");
static const u8 sLoc_342[] = _("UNKNOWN");
static const u8 sLoc_343[] = _("UNKNOWN");
static const u8 sLoc_344[] = _("UNKNOWN");
static const u8 sLoc_345[] = _("UNKNOWN");
static const u8 sLoc_346[] = _("UNKNOWN");
static const u8 sLoc_347[] = _("UNKNOWN");
static const u8 sLoc_348[] = _("UNKNOWN");
static const u8 sLoc_349[] = _("UNKNOWN");
static const u8 sLoc_350[] = _("UNKNOWN");
static const u8 sLoc_351[] = _("UNKNOWN");
static const u8 sLoc_352[] = _("UNKNOWN");
static const u8 sLoc_353[] = _("UNKNOWN");
static const u8 sLoc_354[] = _("UNKNOWN");
static const u8 sLoc_355[] = _("UNKNOWN");
static const u8 sLoc_356[] = _("UNKNOWN");
static const u8 sLoc_357[] = _("UNKNOWN");
static const u8 sLoc_358[] = _("UNKNOWN");
static const u8 sLoc_359[] = _("UNKNOWN");
static const u8 sLoc_360[] = _("UNKNOWN");
static const u8 sLoc_361[] = _("UNKNOWN");
static const u8 sLoc_362[] = _("UNKNOWN");
static const u8 sLoc_363[] = _("UNKNOWN");
static const u8 sLoc_364[] = _("UNKNOWN");
static const u8 sLoc_365[] = _("UNKNOWN");
static const u8 sLoc_366[] = _("UNKNOWN");
static const u8 sLoc_367[] = _("UNKNOWN");
static const u8 sLoc_368[] = _("UNKNOWN");
static const u8 sLoc_369[] = _("UNKNOWN");
static const u8 sLoc_370[] = _("UNKNOWN");
static const u8 sLoc_371[] = _("UNKNOWN");
static const u8 sLoc_372[] = _("UNKNOWN");
static const u8 sLoc_373[] = _("UNKNOWN");
static const u8 sLoc_374[] = _("UNKNOWN");
static const u8 sLoc_375[] = _("UNKNOWN");
static const u8 sLoc_376[] = _("UNKNOWN");
static const u8 sLoc_377[] = _("UNKNOWN");
static const u8 sLoc_378[] = _("UNKNOWN");
static const u8 sLoc_379[] = _("UNKNOWN");
static const u8 sLoc_380[] = _("UNKNOWN");
static const u8 sLoc_381[] = _("UNKNOWN");
static const u8 sLoc_382[] = _("UNKNOWN");
static const u8 sLoc_383[] = _("UNKNOWN");
static const u8 sLoc_384[] = _("UNKNOWN");
static const u8 sLoc_385[] = _("UNKNOWN");
static const u8 sLoc_386[] = _("UNKNOWN");

static const u8 *const sPokemonLocations[] = {
    [0] = sLoc_0,
    [1] = sLoc_1,
    [2] = sLoc_2,
    [3] = sLoc_3,
    [4] = sLoc_4,
    [5] = sLoc_5,
    [6] = sLoc_6,
    [7] = sLoc_7,
    [8] = sLoc_8,
    [9] = sLoc_9,
    [10] = sLoc_10,
    [11] = sLoc_11,
    [12] = sLoc_12,
    [13] = sLoc_13,
    [14] = sLoc_14,
    [15] = sLoc_15,
    [16] = sLoc_16,
    [17] = sLoc_17,
    [18] = sLoc_18,
    [19] = sLoc_19,
    [20] = sLoc_20,
    [21] = sLoc_21,
    [22] = sLoc_22,
    [23] = sLoc_23,
    [24] = sLoc_24,
    [25] = sLoc_25,
    [26] = sLoc_26,
    [27] = sLoc_27,
    [28] = sLoc_28,
    [29] = sLoc_29,
    [30] = sLoc_30,
    [31] = sLoc_31,
    [32] = sLoc_32,
    [33] = sLoc_33,
    [34] = sLoc_34,
    [35] = sLoc_35,
    [36] = sLoc_36,
    [37] = sLoc_37,
    [38] = sLoc_38,
    [39] = sLoc_39,
    [40] = sLoc_40,
    [41] = sLoc_41,
    [42] = sLoc_42,
    [43] = sLoc_43,
    [44] = sLoc_44,
    [45] = sLoc_45,
    [46] = sLoc_46,
    [47] = sLoc_47,
    [48] = sLoc_48,
    [49] = sLoc_49,
    [50] = sLoc_50,
    [51] = sLoc_51,
    [52] = sLoc_52,
    [53] = sLoc_53,
    [54] = sLoc_54,
    [55] = sLoc_55,
    [56] = sLoc_56,
    [57] = sLoc_57,
    [58] = sLoc_58,
    [59] = sLoc_59,
    [60] = sLoc_60,
    [61] = sLoc_61,
    [62] = sLoc_62,
    [63] = sLoc_63,
    [64] = sLoc_64,
    [65] = sLoc_65,
    [66] = sLoc_66,
    [67] = sLoc_67,
    [68] = sLoc_68,
    [69] = sLoc_69,
    [70] = sLoc_70,
    [71] = sLoc_71,
    [72] = sLoc_72,
    [73] = sLoc_73,
    [74] = sLoc_74,
    [75] = sLoc_75,
    [76] = sLoc_76,
    [77] = sLoc_77,
    [78] = sLoc_78,
    [79] = sLoc_79,
    [80] = sLoc_80,
    [81] = sLoc_81,
    [82] = sLoc_82,
    [83] = sLoc_83,
    [84] = sLoc_84,
    [85] = sLoc_85,
    [86] = sLoc_86,
    [87] = sLoc_87,
    [88] = sLoc_88,
    [89] = sLoc_89,
    [90] = sLoc_90,
    [91] = sLoc_91,
    [92] = sLoc_92,
    [93] = sLoc_93,
    [94] = sLoc_94,
    [95] = sLoc_95,
    [96] = sLoc_96,
    [97] = sLoc_97,
    [98] = sLoc_98,
    [99] = sLoc_99,
    [100] = sLoc_100,
    [101] = sLoc_101,
    [102] = sLoc_102,
    [103] = sLoc_103,
    [104] = sLoc_104,
    [105] = sLoc_105,
    [106] = sLoc_106,
    [107] = sLoc_107,
    [108] = sLoc_108,
    [109] = sLoc_109,
    [110] = sLoc_110,
    [111] = sLoc_111,
    [112] = sLoc_112,
    [113] = sLoc_113,
    [114] = sLoc_114,
    [115] = sLoc_115,
    [116] = sLoc_116,
    [117] = sLoc_117,
    [118] = sLoc_118,
    [119] = sLoc_119,
    [120] = sLoc_120,
    [121] = sLoc_121,
    [122] = sLoc_122,
    [123] = sLoc_123,
    [124] = sLoc_124,
    [125] = sLoc_125,
    [126] = sLoc_126,
    [127] = sLoc_127,
    [128] = sLoc_128,
    [129] = sLoc_129,
    [130] = sLoc_130,
    [131] = sLoc_131,
    [132] = sLoc_132,
    [133] = sLoc_133,
    [134] = sLoc_134,
    [135] = sLoc_135,
    [136] = sLoc_136,
    [137] = sLoc_137,
    [138] = sLoc_138,
    [139] = sLoc_139,
    [140] = sLoc_140,
    [141] = sLoc_141,
    [142] = sLoc_142,
    [143] = sLoc_143,
    [144] = sLoc_144,
    [145] = sLoc_145,
    [146] = sLoc_146,
    [147] = sLoc_147,
    [148] = sLoc_148,
    [149] = sLoc_149,
    [150] = sLoc_150,
    [151] = sLoc_151,
    [152] = sLoc_152,
    [153] = sLoc_153,
    [154] = sLoc_154,
    [155] = sLoc_155,
    [156] = sLoc_156,
    [157] = sLoc_157,
    [158] = sLoc_158,
    [159] = sLoc_159,
    [160] = sLoc_160,
    [161] = sLoc_161,
    [162] = sLoc_162,
    [163] = sLoc_163,
    [164] = sLoc_164,
    [165] = sLoc_165,
    [166] = sLoc_166,
    [167] = sLoc_167,
    [168] = sLoc_168,
    [169] = sLoc_169,
    [170] = sLoc_170,
    [171] = sLoc_171,
    [172] = sLoc_172,
    [173] = sLoc_173,
    [174] = sLoc_174,
    [175] = sLoc_175,
    [176] = sLoc_176,
    [177] = sLoc_177,
    [178] = sLoc_178,
    [179] = sLoc_179,
    [180] = sLoc_180,
    [181] = sLoc_181,
    [182] = sLoc_182,
    [183] = sLoc_183,
    [184] = sLoc_184,
    [185] = sLoc_185,
    [186] = sLoc_186,
    [187] = sLoc_187,
    [188] = sLoc_188,
    [189] = sLoc_189,
    [190] = sLoc_190,
    [191] = sLoc_191,
    [192] = sLoc_192,
    [193] = sLoc_193,
    [194] = sLoc_194,
    [195] = sLoc_195,
    [196] = sLoc_196,
    [197] = sLoc_197,
    [198] = sLoc_198,
    [199] = sLoc_199,
    [200] = sLoc_200,
    [201] = sLoc_201,
    [202] = sLoc_202,
    [203] = sLoc_203,
    [204] = sLoc_204,
    [205] = sLoc_205,
    [206] = sLoc_206,
    [207] = sLoc_207,
    [208] = sLoc_208,
    [209] = sLoc_209,
    [210] = sLoc_210,
    [211] = sLoc_211,
    [212] = sLoc_212,
    [213] = sLoc_213,
    [214] = sLoc_214,
    [215] = sLoc_215,
    [216] = sLoc_216,
    [217] = sLoc_217,
    [218] = sLoc_218,
    [219] = sLoc_219,
    [220] = sLoc_220,
    [221] = sLoc_221,
    [222] = sLoc_222,
    [223] = sLoc_223,
    [224] = sLoc_224,
    [225] = sLoc_225,
    [226] = sLoc_226,
    [227] = sLoc_227,
    [228] = sLoc_228,
    [229] = sLoc_229,
    [230] = sLoc_230,
    [231] = sLoc_231,
    [232] = sLoc_232,
    [233] = sLoc_233,
    [234] = sLoc_234,
    [235] = sLoc_235,
    [236] = sLoc_236,
    [237] = sLoc_237,
    [238] = sLoc_238,
    [239] = sLoc_239,
    [240] = sLoc_240,
    [241] = sLoc_241,
    [242] = sLoc_242,
    [243] = sLoc_243,
    [244] = sLoc_244,
    [245] = sLoc_245,
    [246] = sLoc_246,
    [247] = sLoc_247,
    [248] = sLoc_248,
    [249] = sLoc_249,
    [250] = sLoc_250,
    [251] = sLoc_251,
    [252] = sLoc_252,
    [253] = sLoc_253,
    [254] = sLoc_254,
    [255] = sLoc_255,
    [256] = sLoc_256,
    [257] = sLoc_257,
    [258] = sLoc_258,
    [259] = sLoc_259,
    [260] = sLoc_260,
    [261] = sLoc_261,
    [262] = sLoc_262,
    [263] = sLoc_263,
    [264] = sLoc_264,
    [265] = sLoc_265,
    [266] = sLoc_266,
    [267] = sLoc_267,
    [268] = sLoc_268,
    [269] = sLoc_269,
    [270] = sLoc_270,
    [271] = sLoc_271,
    [272] = sLoc_272,
    [273] = sLoc_273,
    [274] = sLoc_274,
    [275] = sLoc_275,
    [276] = sLoc_276,
    [277] = sLoc_277,
    [278] = sLoc_278,
    [279] = sLoc_279,
    [280] = sLoc_280,
    [281] = sLoc_281,
    [282] = sLoc_282,
    [283] = sLoc_283,
    [284] = sLoc_284,
    [285] = sLoc_285,
    [286] = sLoc_286,
    [287] = sLoc_287,
    [288] = sLoc_288,
    [289] = sLoc_289,
    [290] = sLoc_290,
    [291] = sLoc_291,
    [292] = sLoc_292,
    [293] = sLoc_293,
    [294] = sLoc_294,
    [295] = sLoc_295,
    [296] = sLoc_296,
    [297] = sLoc_297,
    [298] = sLoc_298,
    [299] = sLoc_299,
    [300] = sLoc_300,
    [301] = sLoc_301,
    [302] = sLoc_302,
    [303] = sLoc_303,
    [304] = sLoc_304,
    [305] = sLoc_305,
    [306] = sLoc_306,
    [307] = sLoc_307,
    [308] = sLoc_308,
    [309] = sLoc_309,
    [310] = sLoc_310,
    [311] = sLoc_311,
    [312] = sLoc_312,
    [313] = sLoc_313,
    [314] = sLoc_314,
    [315] = sLoc_315,
    [316] = sLoc_316,
    [317] = sLoc_317,
    [318] = sLoc_318,
    [319] = sLoc_319,
    [320] = sLoc_320,
    [321] = sLoc_321,
    [322] = sLoc_322,
    [323] = sLoc_323,
    [324] = sLoc_324,
    [325] = sLoc_325,
    [326] = sLoc_326,
    [327] = sLoc_327,
    [328] = sLoc_328,
    [329] = sLoc_329,
    [330] = sLoc_330,
    [331] = sLoc_331,
    [332] = sLoc_332,
    [333] = sLoc_333,
    [334] = sLoc_334,
    [335] = sLoc_335,
    [336] = sLoc_336,
    [337] = sLoc_337,
    [338] = sLoc_338,
    [339] = sLoc_339,
    [340] = sLoc_340,
    [341] = sLoc_341,
    [342] = sLoc_342,
    [343] = sLoc_343,
    [344] = sLoc_344,
    [345] = sLoc_345,
    [346] = sLoc_346,
    [347] = sLoc_347,
    [348] = sLoc_348,
    [349] = sLoc_349,
    [350] = sLoc_350,
    [351] = sLoc_351,
    [352] = sLoc_352,
    [353] = sLoc_353,
    [354] = sLoc_354,
    [355] = sLoc_355,
    [356] = sLoc_356,
    [357] = sLoc_357,
    [358] = sLoc_358,
    [359] = sLoc_359,
    [360] = sLoc_360,
    [361] = sLoc_361,
    [362] = sLoc_362,
    [363] = sLoc_363,
    [364] = sLoc_364,
    [365] = sLoc_365,
    [366] = sLoc_366,
    [367] = sLoc_367,
    [368] = sLoc_368,
    [369] = sLoc_369,
    [370] = sLoc_370,
    [371] = sLoc_371,
    [372] = sLoc_372,
    [373] = sLoc_373,
    [374] = sLoc_374,
    [375] = sLoc_375,
    [376] = sLoc_376,
    [377] = sLoc_377,
    [378] = sLoc_378,
    [379] = sLoc_379,
    [380] = sLoc_380,
    [381] = sLoc_381,
    [382] = sLoc_382,
    [383] = sLoc_383,
    [384] = sLoc_384,
    [385] = sLoc_385,
    [386] = sLoc_386,
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


static bool8 IsFireRedPokemon(u16 natDex)
{
    // Only Gen 1 (1-151)
    if (natDex < 1 || natDex > 151)
        return FALSE;
        
    // Exclude LeafGreen exclusives:
    switch (natDex)
    {
    case 27: // Sandshrew
    case 28: // Sandslash
    case 37: // Vulpix
    case 38: // Ninetales
    case 69: // Bellsprout
    case 70: // Weepinbell
    case 71: // Victreebel
    case 79: // Slowpoke
    case 80: // Slowbro
    case 120: // Staryu
    case 121: // Starmie
    case 126: // Magmar
    case 127: // Pinsir
        return FALSE;
    }
    
    return TRUE;
}

static bool8 StringStartsWith(const u8 *str, const u8 *prefix)
{
    while (*prefix != 0xFF)
    {
        if (*prefix != *str)
            return FALSE;
        prefix++;
        str++;
    }
    return TRUE;
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
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG0_ON);
        ShowBg(0);
        gMain.state++;
        break;
    case 6:
        if (sTracker == NULL)
        {
            sTracker = AllocZeroed(sizeof(*sTracker));
            sTracker->searchQuery[0] = 0xFF; // empty string
            sTracker->searchFocus = TRUE;    // start focused on search
            sTracker->cursorCol = 0;
        }
        else
        {
            sTracker->state = 0; // Reset state when returning from naming screen
        }
        {
            u32 i;
            for (i = 0; i < 6; i++) sTracker->monIconSpriteIds[i] = 0xFF;
        }
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
        // Ensure standard text palettes are loaded!
        ListMenuLoadStdPalAt(BG_PLTT_ID(1), 1);
        ListMenuLoadStdPalAt(BG_PLTT_ID(2), 2);
        LoadMonIconPalettes();
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
        u32 validCount = 0;
        struct ListMenuTemplate template;
        
        for (i = 0; i < 151; i++) // Max possible to check is 151 now!
        {
            u16 natDex = i + 1;
            if (IsFireRedPokemon(natDex))
            {
                u16 species = NationalPokedexNumToSpecies(natDex);
                u8 *ptr;
                
                if (sTracker->searchQuery[0] != 0xFF)
                {
                    if (!StringStartsWith(gSpeciesNames[species], sTracker->searchQuery))
                        continue;
                }
                
                // Just use empty string so ListMenu doesn't draw anything
                StringCopy(sTracker->itemStrings[validCount], sText_Empty);
                
                sTracker->listItems[validCount].label = sTracker->itemStrings[validCount];
                sTracker->listItems[validCount].index = natDex;
                
                validCount++;
            }
        }

        template = sTrackerListMenuTemplate;
        template.totalItems = validCount;
        template.items = sTracker->listItems;
        template.windowId = sTracker->windowId;
        
        sTracker->totalItems = validCount;

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
    // Empty because we draw everything manually in Tracker_CursorMoveFunc
}

static void CleanupTracker(u8 taskId)
{
    u32 i;
    if (sTracker->listMenuTaskId != 0xFF)
    {
        DestroyListMenuTask(sTracker->listMenuTaskId, NULL, NULL);
        sTracker->listMenuTaskId = 0xFF;
        for (i = 0; i < 4; i++)
        {
            if (sTracker->monIconSpriteIds[i] != 0xFF)
            {
                DestroyMonIcon(&gSprites[sTracker->monIconSpriteIds[i]]);
                sTracker->monIconSpriteIds[i] = 0xFF;
            }
        }
        FreeMonIconPalettes();
    }
    FreeAllWindowBuffers();
    DestroyTask(taskId);
}

static void Task_TrackerMain(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    if (sTracker->searchFocus)
    {
        if (JOY_NEW(DPAD_DOWN) && sTracker->totalItems > 0)
        {
            sTracker->searchFocus = FALSE;
            PlaySE(SE_SELECT);
            Tracker_CursorMoveFunc(0, FALSE, (struct ListMenu *)gTasks[sTracker->listMenuTaskId].data);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sTracker->state = 2; // State 2: Naming screen
        }
        else if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sTracker->state = 1; // Return to field
        }
    }
    else
    {
        u16 scrollOffset, cursorRow;
        ListMenuGetScrollAndRow(sTracker->listMenuTaskId, &scrollOffset, &cursorRow);
        
        if (JOY_NEW(DPAD_UP) && scrollOffset == 0 && cursorRow == 0)
        {
            sTracker->searchFocus = TRUE;
            sTracker->cursorCol = 0;
            PlaySE(SE_SELECT);
            Tracker_CursorMoveFunc(0, FALSE, (struct ListMenu *)gTasks[sTracker->listMenuTaskId].data);
        }
        else
        {
            if (!sTracker->searchFocus)
            {
                if (JOY_NEW(DPAD_RIGHT) && sTracker->cursorCol < 1)
                {
                    sTracker->cursorCol++;
                    PlaySE(SE_SELECT);
                    Tracker_CursorMoveFunc(0, FALSE, (struct ListMenu *)gTasks[sTracker->listMenuTaskId].data);
                }
                else if (JOY_NEW(DPAD_LEFT) && sTracker->cursorCol > 0)
                {
                    sTracker->cursorCol--;
                    PlaySE(SE_SELECT);
                    Tracker_CursorMoveFunc(0, FALSE, (struct ListMenu *)gTasks[sTracker->listMenuTaskId].data);
                }
            }
            ListMenu_ProcessInput(sTracker->listMenuTaskId);
        }

        if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sTracker->state = 1;
        }
    }

    if (sTracker->state != 0 && !gPaletteFade.active)
    {
        if (sTracker->state == 1)
        {
            CleanupTracker(taskId);
            Free(sTracker);
            sTracker = NULL;
            SetMainCallback2(CB2_ReturnToField);
        }
        else if (sTracker->state == 2)
        {
            CleanupTracker(taskId);
            DoNamingScreen(NAMING_SCREEN_BOX, sTracker->searchQuery, 0, 0, 0, CB2_InitPokedexTrackerScreen);
        }
    }
}

static void Tracker_CursorMoveFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    u16 scrollOffset, cursorRow;
    u32 i;
    
    if (sTracker == NULL) return;
    
    if (onInit)
    {
        scrollOffset = 0;
        cursorRow = 0;
    }
    else
    {
        ListMenuGetScrollAndRow(sTracker->listMenuTaskId, &scrollOffset, &cursorRow);
    }

    FillWindowPixelBuffer(sTracker->windowId, PIXEL_FILL(1));

    // Draw Search Bar
    {
        u8 searchText[64];
        StringCopy(searchText, sText_Search);
        StringAppend(searchText, sTracker->searchQuery);
        
        if (sTracker->searchFocus)
            AddTextPrinterParameterized(sTracker->windowId, 1, sText_Cursor, 0, 2, 0xFF, NULL);
            
        AddTextPrinterParameterized(sTracker->windowId, 1, searchText, 16, 2, 0xFF, NULL);
        AddTextPrinterParameterized(sTracker->windowId, 1, sText_StatusColumn, 180, 2, 0xFF, NULL);
    }

    for (i = 0; i < 4; i++)
    {
        u16 listIdx = scrollOffset + i;
        if (sTracker->monIconSpriteIds[i] != 0xFF)
        {
            DestroyMonIcon(&gSprites[sTracker->monIconSpriteIds[i]]);
            sTracker->monIconSpriteIds[i] = 0xFF;
        }

        if (listIdx < sTracker->totalItems)
        {
            u16 natDex = sTracker->listItems[listIdx].index;
            u16 species = NationalPokedexNumToSpecies(natDex);
            u8 text[64];
            u8 *ptr;
            u16 custom_y = 24 + (i * 28);

            // Draw Cursor
            if (!sTracker->searchFocus && i == cursorRow)
            {
                u8 cursorX = (sTracker->cursorCol == 0) ? 0 : 95;
                AddTextPrinterParameterized(sTracker->windowId, 1, sText_Cursor, cursorX, custom_y, 0xFF, NULL);
            }

            // Print Pokemon Name
            AddTextPrinterParameterized(sTracker->windowId, 1, gSpeciesNames[species], 40, custom_y, 0xFF, NULL);

            // Print Location
            if (natDex <= 386)
                AddTextPrinterParameterized(sTracker->windowId, 1, sText_LocationBtn, 105, custom_y, 0xFF, NULL);
            else
                AddTextPrinterParameterized(sTracker->windowId, 1, sText_Unknown, 105, custom_y, 0xFF, NULL);

            // Status Mark
            if (GetSetPokedexFlag(natDex, FLAG_GET_CAUGHT))
            {
                AddTextPrinterParameterized(sTracker->windowId, 1, sText_MarkCaught, 195, custom_y, 0xFF, NULL);
            }
            else
            {
                AddTextPrinterParameterized(sTracker->windowId, 1, sText_MarkNotCaught, 195, custom_y, 0xFF, NULL);
            }

            // Screen X=32, Screen Y=8 (window) + custom_y + 8 (center of 16px text)
            sTracker->monIconSpriteIds[i] = CreateMonIcon(species, SpriteCallbackDummy, 32, 8 + custom_y + 8, 0, 0xFFFFFFFF, FALSE);
        }
    }
    CopyWindowToVram(sTracker->windowId, COPYWIN_GFX);
}
