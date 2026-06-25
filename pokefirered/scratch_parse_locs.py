import re

# Read the file
with open('/Users/sameetmandewalker/Documents/Pokedex-Tracker/pokemon_firered_locations.md', 'r') as f:
    text = f.read()

# We need to map string locations to MAPSEC_ constants.
# Let's define a mapping from strings in the file to MAPSEC_.

mapsec_mapping = {
    'Pallet Town': 'MAPSEC_PALLET_TOWN',
    'Viridian City': 'MAPSEC_VIRIDIAN_CITY',
    'Pewter City': 'MAPSEC_PEWTER_CITY',
    'Pewter Museum': 'MAPSEC_PEWTER_CITY',
    'Cerulean City': 'MAPSEC_CERULEAN_CITY',
    'Vermilion City': 'MAPSEC_VERMILION_CITY',
    'Lavender Town': 'MAPSEC_LAVENDER_TOWN',
    'Celadon City': 'MAPSEC_CELADON_CITY',
    'Celadon Game Corner': 'MAPSEC_CELADON_CITY',
    'Celadon Condominiums': 'MAPSEC_CELADON_CITY',
    'Fuchsia City': 'MAPSEC_FUCHSIA_CITY',
    'Saffron City': 'MAPSEC_SAFFRON_CITY',
    'Fighting Dojo': 'MAPSEC_SAFFRON_CITY',
    'Silph Co.': 'MAPSEC_SAFFRON_CITY',
    'Cinnabar Island': 'MAPSEC_CINNABAR_ISLAND',
    'Cinnabar Island Lab': 'MAPSEC_CINNABAR_ISLAND',
    'Indigo Plateau': 'MAPSEC_INDIGO_PLATEAU',
    
    'Route 1': 'MAPSEC_ROUTE_1',
    'Route 2': 'MAPSEC_ROUTE_2',
    'Route 3': 'MAPSEC_ROUTE_3',
    'Route 4': 'MAPSEC_ROUTE_4',
    'Route 5': 'MAPSEC_ROUTE_5',
    'Route 6': 'MAPSEC_ROUTE_6',
    'Route 7': 'MAPSEC_ROUTE_7',
    'Route 8': 'MAPSEC_ROUTE_8',
    'Route 9': 'MAPSEC_ROUTE_9',
    'Route 10': 'MAPSEC_ROUTE_10',
    'Route 11': 'MAPSEC_ROUTE_11',
    'Route 12': 'MAPSEC_ROUTE_12',
    'Route 13': 'MAPSEC_ROUTE_13',
    'Route 14': 'MAPSEC_ROUTE_14',
    'Route 15': 'MAPSEC_ROUTE_15',
    'Route 16': 'MAPSEC_ROUTE_16',
    'Route 17': 'MAPSEC_ROUTE_17',
    'Route 18': 'MAPSEC_ROUTE_18',
    'Route 19': 'MAPSEC_ROUTE_19',
    'Route 20': 'MAPSEC_ROUTE_20',
    'Route 21': 'MAPSEC_ROUTE_21',
    'Route 22': 'MAPSEC_ROUTE_22',
    'Route 23': 'MAPSEC_ROUTE_23',
    'Route 24': 'MAPSEC_ROUTE_24',
    'Route 25': 'MAPSEC_ROUTE_25',

    'Viridian Forest': 'MAPSEC_VIRIDIAN_FOREST',
    'Mt. Moon': 'MAPSEC_MT_MOON',
    'S.S. Anne': 'MAPSEC_S_S_ANNE',
    'Diglett\'s Cave': 'MAPSEC_DIGLETTS_CAVE',
    'Victory Road': 'MAPSEC_KANTO_VICTORY_ROAD',
    'Pokémon Mansion': 'MAPSEC_POKEMON_MANSION',
    'Safari Zone': 'MAPSEC_KANTO_SAFARI_ZONE',
    'Rock Tunnel': 'MAPSEC_ROCK_TUNNEL',
    'Seafoam Islands': 'MAPSEC_SEAFOAM_ISLANDS',
    'Pokémon Tower': 'MAPSEC_POKEMON_TOWER',
    'Cerulean Cave': 'MAPSEC_CERULEAN_CAVE',
    'Power Plant': 'MAPSEC_POWER_PLANT',

    'Mt. Ember': 'MAPSEC_MT_EMBER',
    'Berry Forest': 'MAPSEC_BERRY_FOREST',
    'Icefall Cave': 'MAPSEC_ICEFALL_CAVE',
    'Pattern Bush': 'MAPSEC_PATTERN_BUSH',
    'Lost Cave': 'MAPSEC_LOST_CAVE',
    'Kindle Road': 'MAPSEC_KINDLE_ROAD',
    'Treasure Beach': 'MAPSEC_TREASURE_BEACH',
    'Cape Brink': 'MAPSEC_CAPE_BRINK',
    'Bond Bridge': 'MAPSEC_BOND_BRIDGE',
    'Water Labyrinth': 'MAPSEC_WATER_LABYRINTH',
    'Five Isle Meadow': 'MAPSEC_FIVE_ISLE_MEADOW',
    'Memorial Pillar': 'MAPSEC_MEMORIAL_PILLAR',
    'Outcast Island': 'MAPSEC_OUTCAST_ISLAND',
    'Green Path': 'MAPSEC_GREEN_PATH',
    'Water Path': 'MAPSEC_WATER_PATH',
    'Ruin Valley': 'MAPSEC_RUIN_VALLEY',
    'Canyon Entrance': 'MAPSEC_CANYON_ENTRANCE',
    'Sevault Canyon': 'MAPSEC_SEVAULT_CANYON',
    'Tanoby Ruins': 'MAPSEC_TANOBY_RUINS',
    'Navel Rock': 'MAPSEC_NAVEL_ROCK',
    'Birth Island': 'MAPSEC_BIRTH_ISLAND',
    
    'One Island': 'MAPSEC_ONE_ISLAND',
    'Two Island': 'MAPSEC_TWO_ISLAND',
    'Three Island': 'MAPSEC_THREE_ISLAND',
    'Four Island': 'MAPSEC_FOUR_ISLAND',
    'Five Island': 'MAPSEC_FIVE_ISLAND',
    'Six Island': 'MAPSEC_SIX_ISLAND',
    'Seven Island': 'MAPSEC_SEVEN_ISLAND',
}

# Find table rows starting with | #
matches = re.findall(r'\|\s*#(\d{3})\s*\|\s*\*\*([^\*]+)\*\*\s*\|\s*[^|]+\s*\|\s*([^|]+)\s*\|', text)

species_data = {}
for m in matches:
    num = int(m[0])
    name = m[1].strip()
    loc_text = m[2].strip()
    species_data[num] = (name, loc_text)

out = "const u8 *const gSpeciesManualLocations[152] = {\n"

for i in range(1, 152):
    if i not in species_data:
        out += f"    [SPECIES_{i}] = NULL,\n"
        continue
    
    name, loc_text = species_data[i]
    if "Unavailable in wild FireRed" in loc_text or "LeafGreen Exclusive" in loc_text or "Unavailable in FireRed" in loc_text:
        out += f"    [SPECIES_{i}] = NULL, // {name} (Exclusive/Unavailable)\n"
        continue
        
    found_mapsecs = set()
    for key, mapsec in mapsec_mapping.items():
        if re.search(r'\b' + re.escape(key) + r'\b', loc_text, re.IGNORECASE):
            found_mapsecs.add(mapsec)
            
    # Additional logic for ranges e.g. Routes 1, 2, 3
    route_matches = re.findall(r'Routes?\s+((?:\d+(?:,\s*|,\s*and\s*|\s*and\s*)*)+)', loc_text, re.IGNORECASE)
    for rm in route_matches:
        nums = re.findall(r'\d+', rm)
        for n in nums:
            if int(n) <= 25:
                found_mapsecs.add(f'MAPSEC_ROUTE_{n}')
                
    if not found_mapsecs:
        # e.g. "All waterways" - let's check what to do
        pass
        
    if found_mapsecs:
        arr_str = ", ".join(sorted(list(found_mapsecs)))
        out += f"    /* SPECIES_{name.upper()} */ (const u8[]) {{ {arr_str}, MAPSEC_NONE }},\n"
    else:
        out += f"    /* SPECIES_{name.upper()} */ NULL, // Needs manual check: {loc_text}\n"

out += "};\n"

with open('/Users/sameetmandewalker/Documents/Pokedex-Tracker/pokefirered/scratch_out.txt', 'w') as f:
    f.write(out)
