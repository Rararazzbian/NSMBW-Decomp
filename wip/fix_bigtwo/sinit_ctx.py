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
for i in range(0,70):
    print(i, t[i][0], t[i][2], t[i][3])
