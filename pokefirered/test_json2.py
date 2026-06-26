import json

with open('src/data/region_map/region_map_sections.json') as f:
    data = json.load(f)

kanto_start = -1
kanto_end = -1
for i, m in enumerate(data['map_sections']):
    if m.get('id') == 'MAPSEC_PALLET_TOWN':
        kanto_start = i
    if m.get('id') == 'MAPSEC_POWER_PLANT':
        kanto_end = i

for m in data['map_sections'][kanto_start:kanto_end+1]:
    if 'x' in m and 'y' in m:
        if m['x'] == 0 or m['y'] == 0:
            print(f"{m.get('id')}: {m['x']}, {m['y']}")
