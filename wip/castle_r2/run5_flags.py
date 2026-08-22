"""DIAGNOSTIC ONLY -- is the target's schedule reachable at all?

Reuses harness.flags_for('d_basesNP') verbatim and mutates exactly one token, so
no mandatory flag can go missing.  Nothing here is a proposal: the module's flags
are fixed by the build and 19 other functions in this TU match with them.  The
question this answers is whether the residual is a SOURCE-shape effect or an
artefact of the optimiser mode.
"""
import sys, os, subprocess, importlib.util
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H
from sweep import target_fn, va

HERE = os.path.join(ROOT, 'wip', 'castle_r2')
SRC = os.path.join(HERE, 'd_a_wm_castle.cpp')
INC = os.path.join(HERE, 'include')

tname, tins = target_fn(0x15faa0)
TN = va.norm(tins)

BASE = H.flags_for('d_basesNP')

VARIANTS = {
    'baseline(-O4,p)': BASE,
    'O4_plain':        [('-O4' if t == '-O4,p' else t) for t in BASE],
    'O4_s':            [('-O4,s' if t == '-O4,p' else t) for t in BASE],
    'O3_p':            [('-O3,p' if t == '-O4,p' else t) for t in BASE],
    'noschedule':      BASE + ['-opt', 'noschedule'],
    'schedule':        BASE + ['-opt', 'schedule'],
    'no_peephole':     BASE + ['-opt', 'nopeephole'],
    'nointrinsics':    BASE + ['-opt', 'nointrinsics'],
    'proc_750':        [('750' if t == 'gekko' else t) for t in BASE],
}

for tag, flags in VARIANTS.items():
    obj = os.path.join(HERE, 'fl_%s.o' % tag.replace(',', '').replace('(', '').replace(')', '').replace('-', ''))
    txt = obj[:-2] + '.txt'
    args = [H.MWCC] + flags + [SRC, '-o', obj]
    for inc in [INC] + H.INCLUDES:
        args += ['-i', inc.replace('/', os.sep)]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    if p.returncode != 0:
        print('%-18s BUILD FAIL: %s' % (tag, (p.stdout + p.stderr).strip().splitlines()[-1:]))
        continue
    H.disasm(obj, txt)
    got = None
    for name, ins in va.functions(txt):
        if 'getKoopaShipStopPos' in name:
            got = va.norm(ins); break
    if got is None:
        print('%-18s fn not found' % tag); continue
    n = max(len(TN), len(got))
    bad = [i for i in range(n) if (TN[i] if i < len(TN) else '') != (got[i] if i < len(got) else '')]
    print('%-18s words=%-3d diffs=%-3d at %s' % (tag, len(got), len(bad), bad))
