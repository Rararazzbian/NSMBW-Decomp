import re
import os

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
TARGET = os.path.join(BASE, 'target_8007E17C.txt')
DRAFT = os.path.join(BASE, 'draft_disasm.txt')


def insn_bytes(path, fname):
    lines = open(path, encoding='utf-8', errors='replace').read().splitlines()
    fn = False
    out = []
    for l in lines:
        if '.fn ' + fname in l or '.fn "' + fname in l:
            fn = True
            continue
        if fn and ('.endfn' in l):
            break
        if fn:
            m = re.search(r'/\* [0-9A-Fa-f]{8} [0-9A-Fa-f]{8}\s+((?:[0-9A-Fa-f]{2} ){3}[0-9A-Fa-f]{2})', l)
            if m:
                out.append(m.group(1).replace(' ', ''))
    return out


for fname in ('ProcMain__17dBgActorManager_cFv', 'createObjList__17dBgActorManager_cFb'):
    t = insn_bytes(TARGET, fname)
    d = insn_bytes(DRAFT, fname)
    print('=== %s === target %d draft %d' % (fname, len(t), len(d)))
    n = min(len(t), len(d))
    shown = 0
    for i in range(n):
        if t[i] != d[i]:
            print('  insn %d: want %s got %s' % (i, t[i], d[i]))
            shown += 1
            if shown > 40:
                break
    if len(t) != len(d):
        print('  LENGTH DIFF: target %d draft %d' % (len(t), len(d)))
