import os, re, json, subprocess

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
objdir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')
D = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')

objs = []
for fn in sorted(os.listdir(objdir)):
    m = re.match(r'auto_00_([0-9A-Fa-f]+)_([a-z]+)\.o$', fn)
    if m:
        objs.append((int(m.group(1),16), m.group(2), fn))
objs.sort()

# get size + section layout for each via dtk
def elf_info(fn):
    out = subprocess.run([D,'elf','info',os.path.join(objdir,fn)],
                         capture_output=True, text=True).stdout
    secs = re.findall(r'\.\w+ \| \w+ \| (0x[0-9A-Fa-f]+)', out)
    return [int(x,16) for x in secs]

for a, sec, fn in objs:
    if not (0x160000 <= a <= 0x170000 or a in (0x86d0,0x43ef8) or 0x176000 <= a <= 0x178000):
        continue
    secs = elf_info(fn)
    print('0x%06X %-8s size=0x%X  %s' % (a, sec, secs[0] if secs else 0, fn))
