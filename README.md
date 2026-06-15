# Pokédex Tracker: FireRed Custom ROM

This project contains a modified version of Pokémon FireRed that features a built-in, native Pokédex Tracker overlay! Instead of running a clunky Node.js script alongside an emulator, this tracker is compiled directly into the `.gba` ROM file itself. 

## Features

- **In-Game Toggle**: Press the **L Button** at any point in the overworld to toggle the tracker UI on or off.
- **Real-Time Data**: Tracks your **Caught** and **Seen** Kanto Pokédex progress using native game functions.
- **Native UI**: Uses the same window templates and fonts as the base game to look like a fully integrated feature.
- **Auto-Refresh**: Automatically refreshes and persists when entering/exiting menus or finishing battles. 

## How to Play

1. Open your Game Boy Advance emulator of choice (e.g., **Visual Boy Advance (VBA)**, mGBA).
2. Load the compiled custom ROM file located at: `pokefirered/pokefirered.gba`.
3. Start the game.
4. **Usage**: Once you are in the overworld and have the Pokédex, simply press the **L Button** to open the tracker HUD. Press **L** again to close it.

## For Developers: Building the ROM

If you wish to modify the C source code and recompile the ROM yourself, follow these steps.

### Prerequisites (macOS)
You must have the `pret/pokefirered` dependencies installed:
```bash
brew install make pkg-config libpng
```
You also need the ARM cross-compilation toolchain (already provided or accessible via `arm-none-eabi-gcc`).

### Compiling
To compile the game, open a terminal in the `pokefirered` directory and run `make`:
```bash
cd pokefirered
export PATH="/opt/homebrew/bin:$PATH"
make -j$(sysctl -n hw.ncpu)
```
If the build is successful, it will generate a new `pokefirered.gba` in the root folder.

### Key Files
The tracker logic resides within these two areas:
* `src/pokedex_tracker.c`: The core C script that allocates the window, pulls data from `GetKantoPokedexCount`, and draws the text.
* `src/overworld.c`: Contains the hooks (`UpdatePokedexTracker` and `PokedexTracker_OnBGReset`) that call the tracker code safely during the main game loop.
