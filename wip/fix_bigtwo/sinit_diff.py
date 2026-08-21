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
print('target', len(t), 'draft', len(d))
diffs=0
first=None
rows=[]
for i,(tt,dd) in enumerate(zip(t,d)):
    if tt[2]!=dd[2]:
        diffs+=1
        if first is None: first=i
        rows.append(i)
print('raw byte diffs', diffs, 'first at', first)
print('diff row indices:', rows)
