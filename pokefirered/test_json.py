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

print(f"Kanto start: {kanto_start}, end: {kanto_end}")
for m in data['map_sections'][kanto_start:kanto_end+1]:
    x = m.get('x', 0)
    y = m.get('y', 0)
    if x == 0 and y == 0:
        print(f"{m.get('id')}: {x}, {y}")

