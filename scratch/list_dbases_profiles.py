import re

with open('include/game/bases/d_profile.hpp') as f:
    text = f.read()

# Look for profiles in d_profile.hpp
# e.g. extern const dProfile::Profile g_profile_...
profiles = re.findall(r'extern\s+const\s+dProfile::Profile\s+(g_profile_\w+)', text)
print(f"Total profiles in d_profile.hpp: {len(profiles)}")

# Check which ones are in d_basesNP (R_2_5_...)
with open('alias_db.txt') as f:
    alias_text = f.read()

dbases_profiles = []
for p in profiles:
    m = re.search(r'(R_2_5_[0-9a-fA-F]+)\s*=\s*' + re.escape(p), alias_text)
    if m:
        r_tag = m.group(1)
        off = int(r_tag.split('_')[3], 16)
        dbases_profiles.append((off, p))

dbases_profiles.sort()
print(f"Total profiles in d_basesNP: {len(dbases_profiles)}")
for off, p in dbases_profiles[:40]:
    print(f"  +0x{off:05x}: {p}")

