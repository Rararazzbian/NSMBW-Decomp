import os, re, json, subprocess

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
D = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')

sl = json.load(open(os.path.join(ROOT, 'slices', 'd_basesNP.json'), encoding='utf-8'))
sources = {}
for s in sl['slices']:
    if s['source'].startswith('d_basesNP'):
        sr = {}
        for sec, rng in s['memoryRanges'].items():
            a, b = rng.split('-')
            sr[sec] = (int(a,16), int(b,16))
        sources[s['source']] = sr

objs = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]+)_([a-z]+)\.o$', fn)
    if m:
        objs.append((int(m.group(1),16), m.group(2), fn))
objs.sort()

def size(fn):
    out = subprocess.run([D,'elf','info',os.path.join(objdir,fn)],
                         capture_output=True, text=True).stdout
    mm = re.search(r'\.text \| code \| (0x[0-9A-Fa-f]+)', out)
    if not mm:
        mm = re.search(r'\.\w+ \| \w+ \| (0x[0-9A-Fa-f]+)', out)
    return int(mm.group(1),16) if mm else 0

result = {}
for a, sec, fn in objs:
    sz = size(fn)
    end = a + sz
    for src, secs in sources.items():
        for sname, (sa, sb) in secs.items():
            if sa <= a and end <= sb:
                result.setdefault(src, {}).setdefault(sname, []).append((a, sz, fn))

for src in sorted(result):
    print('===', src)
    for sname in ['.text', '.rodata', '.data', '.bss', '.ctors', '.dtors']:
        if sname in result[src]:
            for a, sz, fn in sorted(result[src][sname]):
                print('   %-7s 0x%06X size=0x%05X  %s' % (sname, a, sz, fn))
