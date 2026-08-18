import os, re, json

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')

sl = json.load(open(os.path.join(ROOT, 'slices', 'd_basesNP.json'), encoding='utf-8'))
# source -> section -> (start,end)
src_ranges = {}
for s in sl['slices']:
    if s['source'].startswith('d_basesNP'):
        sr = {}
        for sec, rng in s['memoryRanges'].items():
            a, b = rng.split('-')
            sr[sec] = (int(a,16), int(b,16))
        src_ranges[s['source']] = sr

# index (section_start) -> (source, section)
start_index = {}
for src, secs in src_ranges.items():
    for sec, (a, b) in secs.items():
        start_index.setdefault(a, []).append((src, sec))

objs = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]+)_([a-z]+)\.o$', fn)
    if m:
        objs.append((int(m.group(1),16), fn))
objs.sort()

result = {}
for a, fn in objs:
    if a in start_index:
        for src, sec in start_index[a]:
            result.setdefault(src, {}).setdefault(sec, []).append((a, fn))

for src in sorted(result):
    print('===', src)
    for sec in ['.text', '.rodata', '.data', '.bss', '.ctors', '.dtors']:
        if sec in result[src]:
            for a, fn in sorted(result[src][sec]):
                print('   %-7s 0x%06X  %s' % (sec, a, fn))
