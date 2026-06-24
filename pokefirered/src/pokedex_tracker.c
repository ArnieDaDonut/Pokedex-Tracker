#include "global.h"
#include "gflib.h"
#include "task.h"
#include "menu.h"
#include "pokedex.h"
#include "strings.h"
#include "string_util.h"
#include "text_window.h"
#include "pokedex_tracker.h"
#include "pokedex_tracker_screen.h"

extern void CB2_InitPokedexTrackerScreen(void);


void UpdatePokedexTracker(u16 *newKeys, u16 heldKeys)
{
    if ((*newKeys & SELECT_BUTTON) && (heldKeys & B_BUTTON))
    {
        *newKeys &= ~SELECT_BUTTON; // Consume the SELECT press
        SetMainCallback2(CB2_InitPokedexTrackerScreen);
    }
}
