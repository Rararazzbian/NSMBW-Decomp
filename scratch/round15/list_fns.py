"""List functions in the three text objects with sizes, filtered to the ghost span."""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
LO, HI = 0x163620, 0x164230

total = 0
for f in ['t_163620.txt', 't_164180.txt', 't_164204.txt']:
    path = os.path.join(HERE, f)
    names = H.list_functions(path, with_size=True)
    # list_functions doesn't give addresses; walk the file for comment lines instead.
    print('==', f)
    import re
    pending = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            if s.startswith('#'):
                m = re.search(r'\.\w+:0x([0-9A-Fa-f]+) \| (0x[0-9A-Fa-f]+) \| size: (0x[0-9A-Fa-f]+|\d+)', s)
                if m:
                    pending = (int(m.group(2), 16), int(m.group(3), 16))
                continue
            m = re.match(r'^\.fn\s+"?(.+?)"?\s*,\s*(\w+)', s)
            if m and not m.group(1).startswith('gap_'):
                if pending:
                    addr, size = pending
                    mark = '  <== GHOST' if LO <= addr < HI else ''
                    if LO <= addr < HI:
                        total += size
                    print('  %#08x  size %#06x  %s%s' % (addr, size, m.group(1), mark))
                pending = None
print('ghost .text code total: %#x (%d)' % (total, total))
