import re

with open('src/pokedex_tracker_screen.c', 'r') as f:
    content = f.read()

# Fix double sTracker and missing stuff
content = re.sub(r'static struct PokedexTrackerState \*sTracker;\nstatic u16 sTrackerSavedScrollOffset = 0;\nstatic u16 sTrackerSavedCursorRow = 0;\nstatic void CleanupTracker\(u8 taskId\);\nstatic bool8 StringStartsWith\(const u8 \*str, const u8 \*prefix\);\n\nstatic const u8 sText_Empty\[\] = _\(""\);\nstatic const u8 sText_TrackerTitle\[\] = _\("POKéDEX TRACKER"\);', r'static u16 sTrackerSavedScrollOffset = 0;\nstatic u16 sTrackerSavedCursorRow = 0;\nstatic void CleanupTracker(u8 taskId);\nstatic bool8 StringStartsWith(const u8 *str, const u8 *prefix);\n\nstatic const u8 sText_Empty[] = _("");\nstatic const u8 sText_TrackerTitle[] = _("POKéDEX TRACKER");', content)

with open('src/pokedex_tracker_screen.c', 'w') as f:
    f.write(content)
