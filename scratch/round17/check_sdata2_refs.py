import re

# Which sdata2 pool constants are referenced by THIS unit's functions?
# This unit's .text: 0x8007E180..0x8007F79C (first object + sinit + tail thunks)
# Next unit (dBg_ctr_c) starts at 0x8007F7A0.

files = {
    'first_obj': r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\target_8007E17C.txt',
    'sinit': r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17\target_sinit_d_bg_actor_mn.txt',
}

consts = {}
for label, path in files.items():
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            for m in re.finditer(r'@(\d+)_8042C[0-9A-Fa-f]{3}', line):
                consts.setdefault(m.group(1), set()).add(label)

print('Constants referenced by this unit (first obj + sinit):')
for c in sorted(consts, key=int):
    print('  @%s %s' % (c, ','.join(sorted(consts[c]))))
