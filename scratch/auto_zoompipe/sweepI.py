"""Round 6: init() statement-order sweep; execute frozen at S_CST winner."""
import os
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'auto_zoompipe')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

import importlib.util
_spec = importlib.util.spec_from_file_location('Bld', os.path.join(BASE, 'build.py'))
B = importlib.util.module_from_spec(_spec)
sys.modules['B'] = B
_spec.loader.exec_module(B)

EXECUTE_SRC = open(os.path.join(BASE, 's_S_CST.cpp')).read().split('void daZoomPipeBase_c::init')[0]

INIT_TPL = '''void daZoomPipeBase_c::init(u32 param) {
    u32 w = mParam;

@@BODY@@
}
'''

# statement pieces
S0 = "    mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;"
S1 = "    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;"
ST = "    mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;"
CU = "    mCurrent = mSpeed[0];"
CALL = "    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);"

ORDERS = {
    'O1': [S0, S1, CU, ST],
    'O2': [S0, S1, ST, CU],
    'O3': [S0, CU, S1, ST],
    'O4': [S1, S0, ST, CU],
    'O5': [S0, ST, S1, CU],
    'O6': [ST, S0, S1, CU],
    'O7': [S1, S0, CU, ST],
    'O8': [S0, ST, CU, S1],
}

VARIANTS = {}
for tag, stmts in ORDERS.items():
    body = '\n\n'.join(stmts + [CALL])
    VARIANTS[tag] = EXECUTE_SRC + INIT_TPL.replace('@@BODY@@', body)


def score(name):
    path = os.path.join(BASE, 'i_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='i_%s' % name)
    if not txt:
        print('%-4s COMPILE FAIL' % name)
        return False
    ok_i, msg_i = B.harness.diff_fn(B.TARGET, txt, B.FNS[0])
    ok_e, msg_e = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    ni = sum(1 for ln in msg_i.splitlines() if ' want: ' in ln)
    ne = sum(1 for ln in msg_e.splitlines() if ' want: ' in ln)
    print('%-4s init:%s(%d) exec:%s(%d)%s' % (
        name, 'MATCH' if ok_i else 'diff', ni,
        'MATCH' if ok_e else 'diff', ne,
        '  <<< WINNER' if (ok_i and ok_e) else ''))
    return ok_i and ok_e


if __name__ == '__main__':
    winner = None
    for n in sorted(VARIANTS):
        if score(n):
            winner = n
            break
    print('WINNER:', winner)
