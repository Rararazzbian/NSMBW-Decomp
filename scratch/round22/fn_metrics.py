import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
# dtk line: /* ADDR OFFSET  BYTES */\tMNEMONIC ...
INSNS = re.compile(r'^/\* [0-9A-F]{8} [0-9A-F]{8}.*\*/\s*(.*)$')
FRAME = re.compile(r'^stwu r1, -(0x[0-9A-Fa-f]+)\(r1\)')
SAVE = re.compile(r'^bl (_savegpr_\d+)')

def fn_metrics(path):
    """name -> (length, frame, savegpr) from a dtk disasm"""
    out = {}
    cur = None
    body = []
    for line in open(path, encoding='utf-8', errors='replace'):
        m = FN_START.match(line.strip())
        if m:
            if cur is not None:
                out[cur] = metrics(body)
            cur = m.group(1).strip().strip('"')
            body = []
            continue
        if cur is not None:
            body.append(line)
    if cur is not None:
        out[cur] = metrics(body)
    return out

def metrics(body):
    frame = None
    save = None
    n = 0
    for line in body:
        a = INSNS.match(line)
        if not a:
            continue
        insn = a.group(1).strip()
        if not insn or insn.startswith('.'):
            continue  # directives (.4byte/.word/...) are data, not instructions
        n += 1
        m = FRAME.match(insn)
        if m and frame is None:
            frame = m.group(1)
        m = SAVE.match(insn)
        if m:
            save = m.group(1)
    return (n, frame, save)

if __name__ == '__main__':
    which = sys.argv[1]
    if which == 'mng':
        tgt = os.path.join(ROOT, 'scratch', 'round22', 'target_8007E17C.txt')
        drf = os.path.join(ROOT, 'scratch', 'round22', 'draft_disasm.txt')
        names = ['__ct__17dBgActorManager_cFv', '__dt__17dBgActorManager_cFv',
                 'initialize__17dBgActorManager_cFv', 'create__17dBgActorManager_cFv',
                 'CreateHeap__17dBgActorManager_cFv', 'execute__17dBgActorManager_cFv',
                 'ProcMain__17dBgActorManager_cFv',
                 'addObj__17dBgActorManager_cFUsUsUsUc',
                 'createObjList__17dBgActorManager_cFb',
                 'init__Q217dBgActorManager_c7BgObj_cFv',
                 'clear__Q217dBgActorManager_c7BgObj_cFv',
                 'set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc',
                 'createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c',
                 'deleteActor__Q217dBgActorManager_c7BgObj_cFv',
                 'getOffset__Q217dBgActorManager_c7BgObj_cFv',
                 'getSize__Q217dBgActorManager_c7BgObj_cFv']
    else:
        tgt = os.path.join(ROOT, 'scratch', 'round22', 'd_bg_ctr', 'target.txt')
        drf = os.path.join(ROOT, 'scratch', 'round22', 'd_bg_ctr', 'draft_disasm.txt')
        names = None
    tm, dm = fn_metrics(tgt), fn_metrics(drf)
    if names is None:
        names = [n for n in tm if not n.startswith('gap_') and not n.startswith('pad_')]

    # draft emits stubbed free functions under mangled names (fn_8007XXXX__FP...);
    # map target fn_8007XXXX -> first draft fn whose name starts with the same tag.
    def draft_metric(n):
        if n in dm:
            return dm[n]
        if n.startswith('fn_'):
            tag = n.split('__')[0] if '__' in n else n
            for dn, d in dm.items():
                if dn.startswith(tag + '__') or dn == tag:
                    return d
        return None

    rows = []
    for n in names:
        t = tm.get(n)
        d = draft_metric(n)
        rows.append((n, t, d))
        t0 = t[0] if t else '?'
        d0 = d[0] if d else '?'
        tf = t[1] if t and t[1] else '-'
        ts = t[2] if t and t[2] else '-'
        df = d[1] if d and d[1] else '-'
        ds = d[2] if d and d[2] else '-'
        print('%-74s T:%5s f%-5s s%-13s | D:%5s f%-5s s%-13s' %
              (n[:74], t0, tf, ts, d0, df, ds))

    if which != 'mng':
        print()
        print('ranked by target length (desc):')
        for n, t, d in sorted(rows, key=lambda r: -(r[1][0] if r[1] else 0)):
            t0 = t[0] if t else '?'
            print('%-56s %5s words' % (n[:56], t0))
