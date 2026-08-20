import json
import re
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dump_epilogue as DE

hits = json.load(open(os.path.join(os.path.dirname(__file__), 'small_mismatches.json'),
                       encoding='utf-8'))

R3_WRITE = re.compile(r'^\s*[a-zA-Z_.]+\s+r3\s*,')
RESTORE = re.compile(r'^\s*l\w\w?\s+r\d+,\s*0x[0-9A-Fa-f]+\(r1\)')


def get_body(unit, addr_hex):
    unit_dir = os.path.join(DE.WM_UNITS, unit)
    paths = DE.find_target_files(unit_dir)
    if unit == 'agent_castle':
        paths += DE.CASTLE_TARGET_OBJS_TXT
    want = int(addr_hex, 16)
    for p in paths:
        text = open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^\.fn (\S+?), \w+\n(.*?)^\.endfn', text, re.M | re.S):
            body = m.group(2)
            addrs = re.findall(r'/\* ([0-9A-Fa-f]{8}) ', body)
            if not addrs or int(addrs[0], 16) != want:
                continue
            lines = []
            for line in body.splitlines():
                mm = re.search(r'\*/\s*(.+)$', line)
                if mm:
                    lines.append(mm.group(1).strip())
            return lines
    return None


flagged = []
for h in hits:
    body = get_body(h['unit'], '%08x' % h['addr'])
    if not body:
        continue
    mtlr_idx = None
    for i, l in enumerate(body):
        if l.startswith('mtlr'):
            mtlr_idx = i
    if mtlr_idx is None:
        continue
    restore_idx = [i for i in range(mtlr_idx) if RESTORE.match(body[i])]
    if not restore_idx:
        continue
    first_restore = restore_idx[0]
    r3write_idx = [i for i in range(first_restore, mtlr_idx) if R3_WRITE.match(body[i])]
    if r3write_idx:
        flagged.append((h, body, mtlr_idx, r3write_idx))

print('functions with a register-restore-then-r3-write-then-mtlr epilogue shape:', len(flagged))
for h, body, mtlr_idx, r3write_idx in flagged:
    print()
    print('---', h['unit'], hex(h['addr']), h['target_name'],
          'target_len=%d draft~%s(%d) delta=%+d' % (
              h['target_len'], h['draft_name'], h['draft_len'], h['delta']))
    lo = max(0, r3write_idx[0] - 2)
    for i in range(lo, min(mtlr_idx + 3, len(body))):
        marker = ' <== R3 WRITE' if i in r3write_idx else ''
        print('    ', body[i], marker)
