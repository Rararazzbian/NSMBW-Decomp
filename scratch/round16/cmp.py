"""Compare draft object against target disassembly by raw instruction bytes.

The target spans three split objects. We concatenate their disassembly
functions into one map keyed by normalised name, then compare each target
function in the unit against the draft's same-named function by raw bytes.
"""
import sys, os, re
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round16'
TARGETS = [
    os.path.join(ROOT, 'auto_03_800B9098_text.txt'),
    os.path.join(ROOT, 'auto_sinit__d_iggy_wan_ku_text.txt'),
    os.path.join(ROOT, 'auto_03_800BAB04_text.txt'),
]
DRAFT = os.path.join(ROOT, 'd_iggy_wan_kusari.txt')

# Raw instruction words, preserving order, per function. Returns dict name->[words].
INSN_WORD = re.compile(r'^/\*\s*\S+\s+\S+\s+'
                       r'((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})'
                       r'\s*\*/')

def raw_words(path):
    out = {}
    cur = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = harness.FN_START.match(line.strip())
            if m:
                name = harness.norm_name(m.group(1))
                if not name.startswith('gap_') and not name.startswith('pad_'):
                    cur = []
                    out[name] = cur
                else:
                    cur = None
                continue
            if harness.FN_END.match(line.strip()):
                cur = None
                continue
            if cur is not None:
                mw = INSN_WORD.match(line)
                if mw:
                    cur.append(''.join(mw.group(1).split()))
    return out

def norm_local_branch(words):
    # local branches are PC-relative; identical code -> identical words.
    return words

target = {}
for t in TARGETS:
    for k, v in raw_words(t).items():
        target[k] = v
draft = raw_words(DRAFT)

# The unit's function list, in address order (from symbol map).
unit_names = [
    'create__16dIggyWanKusari_cFi',
    'allocate__16dIggyWanKusari_cFv',
    'execute__16dIggyWanKusari_cFv',
    'calcMdl__16dIggyWanKusari_cFv',
    'draw__16dIggyWanKusari_cFv',
    'remove__16dIggyWanKusari_cFv',
    'make_kusari__16dIggyWanKusari_cFv',
    'createMdl__16dIggyWanKusari_cFv',
    'init__16dIggyWanKusari_cFv',
    'getLength__16dIggyWanKusari_cCFv',
    'setAlphaForKameckMagic__16dIggyWanKusari_cFUc',
    'calcTightRate__16dIggyWanKusari_cFv',
    'ready__16dIggyWanKusari_cFv',
    'normal__16dIggyWanKusari_cFv',
    'tight__16dIggyWanKusari_cFv',
    'release__16dIggyWanKusari_cFv',
    'initializeState_Ready__16dIggyWanKusari_cFv',
    'finalizeState_Ready__16dIggyWanKusari_cFv',
    'executeState_Ready__16dIggyWanKusari_cFv',
    'initializeState_Normal__16dIggyWanKusari_cFv',
    'finalizeState_Normal__16dIggyWanKusari_cFv',
    'executeState_Normal__16dIggyWanKusari_cFv',
    'initializeState_Tight__16dIggyWanKusari_cFv',
    'finalizeState_Tight__16dIggyWanKusari_cFv',
    'executeState_Tight__16dIggyWanKusari_cFv',
    'initializeState_Release__16dIggyWanKusari_cFv',
    'finalizeState_Release__16dIggyWanKusari_cFv',
    'executeState_Release__16dIggyWanKusari_cFv',
    'initializeState_Collapse__16dIggyWanKusari_cFv',
    'finalizeState_Collapse__16dIggyWanKusari_cFv',
    'executeState_Collapse__16dIggyWanKusari_cFv',
    'initializeState_Dead__16dIggyWanKusari_cFv',
    'finalizeState_Dead__16dIggyWanKusari_cFv',
    'executeState_Dead__16dIggyWanKusari_cFv',
    'finalizeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c',
    'executeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c',
    'initializeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c',
    '__sinit_\\d_iggy_wan_kusari_cpp',
    '__dt__31sFStateID_c<16dIggyWanKusari_c>Fv',
    'isSameName__31sFStateID_c<16dIggyWanKusari_c>CFPCc',
    'createMdl__21dIggyWanKusariPiece_cFR16mHeapAllocator_c',
    'calcMdl__21dIggyWanKusariPiece_cFv',
    'draw__21dIggyWanKusariPiece_cFv',
    'calcForDemo__21dIggyWanKusariPiece_cFv',
    'calcPosAngle__21dIggyWanKusariPiece_cFP8dActor_c',
    'collapseMove__21dIggyWanKusariPiece_cFv',
    'setCollapseSpeed__21dIggyWanKusariPiece_cFi',
]

print(f'{"MATCH":5} {"TARGET":>6} {"DRAFT":>6}  name')
match = 0
for n in unit_names:
    t = target.get(n)
    d = draft.get(n)
    if t is None:
        print(f'{"-":5} {"MISSING":>6} {"":>6}  {n} (target not found)')
        continue
    tl = len(t)
    dl = len(d) if d is not None else 0
    ok = (d is not None) and (t == d)
    tag = 'MATCH' if ok else ('LEN' if tl == dl else 'DIFF')
    if ok:
        match += 1
    print(f'{tag:5} {tl:6d} {dl:6d}  {n}')
print(f'\n{len(unit_names)} functions, {match} byte-exact, {len(unit_names)-match} differing')
