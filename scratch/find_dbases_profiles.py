import re

# Parse alias_db.txt for module 2 (d_basesNP)
aliases_2 = {}
with open('alias_db.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            k, v = line.split('=', 1)
            k = k.strip()
            v = v.strip()
            if k.startswith('R_2_'):
                parts = k.split('_') # R, 2, sec_idx, hex_offset
                sec_idx = int(parts[2])
                offset = int(parts[3], 16)
                aliases_2[(sec_idx, offset)] = v

print(f"Total aliases for d_basesNP in alias_db.txt: {len(aliases_2)}")

# Let's list all profile entries in .data (sec_idx 5)
profiles = {off: name for (sec, off), name in aliases_2.items() if sec == 5 and 'profile' in name.lower()}
print(f"Total profile entries in d_basesNP: {len(profiles)}")
for off in sorted(profiles):
    print(f"  .data +0x{off:05x}: {profiles[off]}")

