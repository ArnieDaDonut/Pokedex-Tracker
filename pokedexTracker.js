/**
 * Pokedex Tracker Module for Pokemon FireRed
 * 
 * This module provides a clean architecture for tracking Pokedex progress,
 * separating the data layer (Pokemon stats/locations) from the tracking logic.
 * It's designed to be easily integrated with a ROM memory reader.
 */

// --- Data Layer ---

class Pokemon {
  /**
   * @param {number} id - National Pokedex number
   * @param {string} name - Pokemon name
   * @param {Array<string>} locationData - Array of strings describing encounter locations
   */
  constructor(id, name, locationData = []) {
    this.id = id;
    this.name = name;
    this.locationData = locationData;
    this.caught = false; // Initial state
  }

  // Helper to mark as caught
  markCaught() {
    this.caught = true;
  }
}

// Sample FireRed Database
const FIRE_RED_POKEMON_DATABASE = [
  new Pokemon(1, 'Bulbasaur', ['Pallet Town (Gift)']),
  new Pokemon(4, 'Charmander', ['Pallet Town (Gift)']),
  new Pokemon(7, 'Squirtle', ['Pallet Town (Gift)']),
  new Pokemon(10, 'Caterpie', ['Route 2', 'Route 24', 'Route 25', 'Viridian Forest']),
  new Pokemon(13, 'Weedle', ['Route 2', 'Route 24', 'Route 25', 'Viridian Forest']),
  new Pokemon(16, 'Pidgey', ['Route 1', 'Route 2', 'Route 3', 'Route 5', 'Route 6', 'Route 7', 'Route 8']),
  new Pokemon(19, 'Rattata', ['Route 1', 'Route 2', 'Route 4', 'Route 9', 'Route 16', 'Route 21', 'Route 22']),
  new Pokemon(25, 'Pikachu', ['Viridian Forest', 'Power Plant']),
  new Pokemon(39, 'Jigglypuff', ['Route 3']),
  new Pokemon(41, 'Zubat', ['Mt. Moon', 'Rock Tunnel', 'Seafoam Islands', 'Victory Road']),
];

// --- Tracking Logic ---

class PokedexTracker {
  constructor(pokemonDatabase) {
    // Map for O(1) lookups by ID
    this.database = new Map(pokemonDatabase.map(p => [p.id, p]));
    this.pollingInterval = null;
  }

  get caughtPokemon() {
    return Array.from(this.database.values()).filter(p => p.caught);
  }

  get neededPokemon() {
    return Array.from(this.database.values()).filter(p => !p.caught);
  }

  /**
   * EXTENSION POINT: ROM Memory Integration
   * 
   * Simulate reading caught flags from the FireRed ROM save RAM or live memory.
   * In Gen 3 (FireRed), Pokedex caught flags are stored in a bit array.
   * This method parses that bit array and updates the internal state.
   * 
   * @param {Array<number>} caughtIds - Array of Pokemon IDs read from memory as caught
   * @returns {boolean} true if new Pokemon were caught since last update
   */
  updateFromMemory(caughtIds) {
    let stateChanged = false;
    for (const id of caughtIds) {
      if (this.database.has(id)) {
        const pokemon = this.database.get(id);
        if (!pokemon.caught) {
          pokemon.markCaught();
          stateChanged = true;
        }
      }
    }
    return stateChanged;
  }

  /**
   * Generates hints for a few needed Pokemon to help the player.
   * @param {number} limit - Maximum number of hints to generate
   * @returns {Array<{name: string, locations: Array<string>}>}
   */
  generateLocationHints(limit = 3) {
    const needed = this.neededPokemon;
    return needed.slice(0, limit).map(p => ({
      name: p.name,
      locations: p.locationData
    }));
  }

  /**
   * Generates a real-time HTML overlay snippet for an emulator (like VBA).
   * This can be written to a file that OBS or a browser overlay reads.
   */
  renderOverlay() {
    const caught = this.caughtPokemon;
    const total = this.database.size;
    const progress = Math.round((caught.length / total) * 100);
    const hints = this.generateLocationHints(3);

    const hintsHtml = hints.map(h => `<li><strong>${h.name}</strong>: ${h.locations.join(', ')}</li>`).join('');
    // Show up to the last 3 caught Pokemon
    const caughtHtml = caught.slice(-3).map(p => `<li>#${p.id.toString().padStart(3, '0')} ${p.name}</li>`).join('');

    const htmlSnippet = `
<div class="pokedex-overlay" style="font-family: Arial, sans-serif; background: rgba(0,0,0,0.8); color: white; padding: 15px; border-radius: 8px; width: 300px;">
  <h2 style="margin-top: 0; border-bottom: 2px solid #e3350d; padding-bottom: 5px;">Pokédex Tracker</h2>
  <div class="progress">
    <strong>Progress:</strong> ${caught.length} / ${total} (${progress}%)
  </div>
  
  <h3 style="color: #4caf50; margin-bottom: 5px;">Recently Caught</h3>
  <ul style="margin-top: 0; padding-left: 20px; font-size: 0.9em;">
    ${caughtHtml || '<li>None yet</li>'}
  </ul>

  <h3 style="color: #ff9800; margin-bottom: 5px;">Current Targets</h3>
  <ul style="margin-top: 0; padding-left: 20px; font-size: 0.9em;">
    ${hintsHtml || '<li>Pokédex Complete!</li>'}
  </ul>
</div>
    `;

    // In a real integration, write to file for OBS to load:
    require('fs').writeFileSync('overlay.html', htmlSnippet);

    // Clear console for live updates
    console.clear();
    console.log("=== REAL-TIME OVERLAY UPDATED ===");
    console.log(htmlSnippet);
  }

  /**
   * Polls VBA Memory at a given interval to check for new caught Pokemon.
   * Uses a mock array to simulate memory changing over time.
   * @param {number} interval - Polling interval in milliseconds
   */
  pollVBAMemory(interval = 2000) {
    console.log(`[VBA Hook] Starting memory polling every ${interval}ms...`);

    // Mock memory states representing progression over time
    const memoryStates = [
      [],                        // Initial state
      [4],                       // Caught Charmander
      [4, 16],                   // Caught Pidgey
      [4, 16, 19],               // Caught Rattata
      [4, 16, 19, 25]            // Caught Pikachu
    ];

    let currentStateIndex = 0;

    this.pollingInterval = setInterval(() => {
      if (currentStateIndex < memoryStates.length) {
        const currentMemory = memoryStates[currentStateIndex];

        // Update tracker with simulated RAM read
        const stateChanged = this.updateFromMemory(currentMemory);

        // Only render the overlay if something actually changed (or on first run)
        if (stateChanged || currentStateIndex === 0) {
          this.renderOverlay();
        }

        currentStateIndex++;
      } else {
        console.log("\\n[VBA Hook] Simulation complete. Stopping polling.");
        clearInterval(this.pollingInterval);
      }
    }, interval);
  }
}

// --- Demonstration / Usage Example ---

// Only run the demo if this file is executed directly (not imported)
if (require.main === module) {
  console.log('Initializing Pokedex Tracker with FireRed database...');
  const tracker = new PokedexTracker(FIRE_RED_POKEMON_DATABASE);

  // Start the polling simulation to generate real-time HTML overlay output
  tracker.pollVBAMemory(2000);
}

module.exports = {
  Pokemon,
  PokedexTracker
};
