import re
import os

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
TARGET = os.path.join(BASE, 'target_8007E17C.txt')
DRAFT = os.path.join(BASE, 'draft_disasm.txt')


def insn_bytes(path, fname):
    """Extract the instruction-word hex bytes of an .fn block from a dtk disasm."""
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


for fname in ('initialize__17dBgActorManager_cFv', 'execute__17dBgActorManager_cFv',
              'addObj__17dBgActorManager_cFUsUsUsUc',
              'createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c'):
    t = insn_bytes(TARGET, fname)
    d = insn_bytes(DRAFT, fname)
    print('=== %s ===' % fname)
    print('  target %d insns, draft %d insns' % (len(t), len(d)))
    if t == d:
        print('  RAW BYTES IDENTICAL')
    else:
        n = min(len(t), len(d))
        shown = 0
        for i in range(n):
            if t[i] != d[i]:
                print('  insn %d: want %s got %s' % (i, t[i], d[i]))
                shown += 1
                if shown > 25:
                    break
        if len(t) != len(d):
            print('  LENGTH DIFF: target %d draft %d' % (len(t), len(d)))
