# FireRed Pokédex Tracker

This is an in-game interactive Pokédex Tracker overlay designed for the [pokeemerald / pokefirered](https://github.com/pret/pokefirered) decompilation project. It allows players to press `SELECT + B` in the overworld to view a checklist of caught Pokémon and locations of uncaught Pokémon.

## Installation Instructions

1. **Copy Files**: Copy the `src/pokedex_tracker.c` and `include/pokedex_tracker.h` files into your `pokefirered` decompilation project's respective directories.
2. **Update Makefile**: Open your project's `Makefile` and add `src/pokedex_tracker.o` to your `OBJS` list so it gets compiled.
3. **Hook the Input**: Open `src/field_control_avatar.c` in your project and include the header at the top:
   ```c
   #include "pokedex_tracker.h"
   ```
4. **Modify the Input Loop**: Still in `src/field_control_avatar.c`, find the `ProcessPlayerFieldInput` function and insert the `SELECT + B` hook right before the `StartMenu` or `RegisteredKeyItem` checks (around line 290):
   ```c
       if (input->pressedSelectButton && JOY_HELD(B_BUTTON))
       {
           gFieldInputRecord.pressedSelectButton = TRUE;
           ShowPokedexTracker();
           return TRUE;
       }
   ```
5. **Compile**: Run `make` in your `pokefirered` directory to build your modified `.gba` ROM!

## Architecture

- `gPokedexTrackerData`: Contains the 151 entries for Gen I, defining locations, encounter rates, and encounter methods.
- `ShowPokedexTracker()`: The entry point function that initializes VRAM, BGs, and launches the UI State Machine task.
