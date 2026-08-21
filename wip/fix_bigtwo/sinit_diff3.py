import re
def parse_one(path, key):
    lines = []
    cur = None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+"?([^\s,"]+)', line)
        if m:
            cur = m.group(1)
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur == key:
            mi = re.match(r'/\* ([0-9A-F]+)\s+([0-9A-F]+)\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                lines.append((mi.group(1), mi.group(2), mi.group(3).strip(), mi.group(4).strip()))
    return lines

key = r'__sinit_\d_line_mng_cpp'
t = parse_one('wip/line_mng_shared/target.txt', key)
d = parse_one('wip/fix_bigtwo/_tally/d.txt', key)

group_starts = [8] + list(range(59, 1193, 47))
hexpat = re.compile(r'0x([0-9a-fA-F]+)\(r28\)|r28,\s*0x([0-9a-fA-F]+)')
for gi, gs in enumerate(group_starts):
    deltas = []
    for off in [0,1,2,3,7,8,12]:
        i = gs+off
        if i >= len(t) or i >= len(d):
            continue
        ttxt = t[i][3]
        dtxt = d[i][3]
        mt = hexpat.search(ttxt)
        md = hexpat.search(dtxt)
        if mt and md and 'r28' in ttxt:
            tv = int(mt.group(1) or mt.group(2), 16)
            dv = int(md.group(1) or md.group(2), 16)
            deltas.append(dv - tv)
    print(f'group {gi} start={gs} deltas={deltas}')
