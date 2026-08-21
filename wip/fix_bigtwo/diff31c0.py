import re, sys

def parse_one(path, key):
    lines = []
    cur = None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur == key:
            mi = re.match(r'/\* ([0-9A-F]+)\s+([0-9A-F]+)\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                lines.append((mi.group(1), mi.group(2), mi.group(3).strip(), mi.group(4).strip()))
    return lines

t = parse_one('wip/line_mng_shared/target.txt', 'fn_800C31C0')
d = parse_one('wip/fix_bigtwo/_tally/d.txt', 'fn_800C31C0__FP10dLineMng_c')

print('target len', len(t), 'draft len', len(d))

n = max(len(t), len(d))
diffcount = 0
for i in range(n):
    trow = t[i] if i < len(t) else None
    drow = d[i] if i < len(d) else None
    tb = trow[2] if trow else None
    db = drow[2] if drow else None
    same = (tb == db)
    if not same:
        diffcount += 1
        taddr = trow[0] if trow else '----'
        toff = trow[1] if trow else '----'
        ttxt = trow[3] if trow else ''
        daddr = drow[0] if drow else '----'
        dtxt = drow[3] if drow else ''
        print(f'{i:4d} T[{taddr} {toff}] {tb!s:14} {ttxt:40} | D[{daddr}] {db!s:14} {dtxt}')
print('total differing rows', diffcount)
