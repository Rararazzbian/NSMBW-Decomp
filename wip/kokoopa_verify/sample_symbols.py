import sys, re
sys.path.insert(0, 'tools/auto_decomp')
import harness

target_path = 'wip/kokoopa_verify/target_combined.txt'
draft_path = 'wip/kokoopa_verify/draft.txt'

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')

def raw_extract(path, name):
    want = harness.norm_name(name)
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                body = [] if harness.norm_name(m.group(1)) == want else None
                continue
            if s.startswith('.endfn'):
                if body is not None:
                    return body
                continue
            if body is not None:
                mi = INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return body

SAMPLE = [
    # smallest / trivial constant-returning functions (low collision risk baseline)
    'moveAdjust_HIO__18dEnTorideKokoopa_cFv',
    'getDrawScale__18dEnTorideKokoopa_cFv',
    'getJumpGravity__18dEnTorideKokoopa_cFv',
    'defaultDirAngle__18dEnTorideKokoopa_cFv',
    'checkGetUp__18dEnTorideKokoopa_cCFv',
    # "free" state-declaration functions
    'baseID_Jump_St<10sStateID_c>__Fv_RC12sStateIDIf_c',
    'baseID_AttackSearch<10sStateID_c>__Fv_RC12sStateIDIf_c',
    'baseID_ShellOut<10sStateID_c>__Fv_RC12sStateIDIf_c',
    'baseID_DemoWait<9dEnBoss_c>__Fv_RC12sStateIDIf_c',
    'baseID_DieFire<9dEnBoss_c>__Fv_RC12sStateIDIf_c',
    '__dt__33sFStateID_c<18dEnTorideKokoopa_c>Fv',
    '__dt__40sFStateVirtualID_c<18dEnTorideKokoopa_c>Fv',
    'superID__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv',
    'number__40sFStateVirtualID_c<18dEnTorideKokoopa_c>CFv',
    'isSameName__33sFStateID_c<18dEnTorideKokoopa_c>CFPCc',
    'initializeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c',
    'executeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c',
    'finalizeState__33sFStateID_c<18dEnTorideKokoopa_c>CFR18dEnTorideKokoopa_c',
    # authored logic functions with real member/global references
    'damageProc__18dEnTorideKokoopa_cFv',
    'damageSVo__18dEnTorideKokoopa_cFv',
    'damageLVo__18dEnTorideKokoopa_cFv',
    'deadProc__18dEnTorideKokoopa_cFv',
    'calcRootJntPos__18dEnTorideKokoopa_cFv',
    'calcShellJntPos__18dEnTorideKokoopa_cFv',
    'finalUpdate__18dEnTorideKokoopa_cFv',
    'draw__18dEnTorideKokoopa_cFv',
]

SYMISH = re.compile(r'@|"|->|\b(?:bl|lis|lwz|stw|addi)\b.*[A-Za-z_]')

for name in SAMPLE:
    t = raw_extract(target_path, name)
    if t is None:
        # __sinit / not in combined file naming — skip, handled separately
        print('=== %s === TARGET NOT FOUND in combined (skipping)' % name)
        continue
    d = raw_extract(draft_path, name)
    if d is None:
        print('=== %s === DRAFT NOT FOUND (not emitted)' % name)
        continue
    print('=== %s === (target %d instr, draft %d instr)' % (name, len(t), len(d)))
    any_sym = False
    for i in range(max(len(t), len(d))):
        tl = t[i] if i < len(t) else '<none>'
        dl = d[i] if i < len(d) else '<none>'
        if SYMISH.search(tl) or SYMISH.search(dl):
            any_sym = True
            marker = '  ' if tl == dl else '<>'
            print('  %s target: %-55s draft: %s' % (marker, tl, dl))
    if not any_sym:
        print('  (no symbol/data references in this function -- pure register/immediate ops)')
    print()
