import re
import json

def parse_fns(path):
    text = open(path, encoding='utf-8', errors='replace').read()
    out = []
    pat = re.compile(
        r'# \.text:0x[0-9A-Fa-f]+ \| (0x[0-9A-Fa-f]+) \| size: (0x[0-9A-Fa-f]+)\n'
        r'(?:#[^\n]*\n)*\.fn\s+"?([^,\n"]+)"?\s*,\s*\w+', re.M)
    for m in pat.finditer(text):
        addr = int(m.group(1), 16)
        size = int(m.group(2), 16)
        name = m.group(3)
        out.append((addr, size, name))
    return out

f1 = parse_fns('scratch/gemini_round16/auto_03_800A8710_text.txt')
f2 = parse_fns('scratch/gemini_round16/auto_03_800B03D8_text.txt')
allfns = f1 + f2
print('total parsed:', len(allfns))

TU_START = 0x800A8710
TU_END = 0x800B0A20
in_range = [(a, s, n) for a, s, n in allfns if TU_START <= a < TU_END and not n.startswith('gap_')]
print('in TU range (excl gap):', len(in_range), 'total bytes:', sum(s for a, s, n in in_range))

with open('wip/kokoopa_verify/target_fns_in_range.json', 'w', encoding='utf-8') as fh:
    json.dump(in_range, fh)

for a, s, n in in_range[:15]:
    print(hex(a), s, n)
print('...')
for a, s, n in in_range[-15:]:
    print(hex(a), s, n)
