"""Round 8: init statement-order sweep around the f1/f2 arg-register insight."""
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

ALL_SRC = open(os.path.join(BASE, 'j_W5.cpp')).read()
INCLUDE_LINE = '#include <game/bases/d_a_zoom_pipe_base.hpp>'
EXECUTE_SRC = '\n\n' + ALL_SRC[ALL_SRC.index('int daZoomPipeBase_c::execute()'):].rstrip() + '\n'

E_ST = "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;"
E_S0 = "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;"
E_S1 = "mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;"
E_CU = "mCurrent = mSpeed[0];"
E_CH = "mSpeed[0] = mCurrent = ((w >> 4) & 0xF) * 16.0f + 16.0f;"
CALL = "fn_80045A10(this, param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);"

INIT_TPL = 'void daZoomPipeBase_c::init(u32 param) {\n    u32 w = mParam;\n\n@@BODY@@\n}\n'


def j(*stmts):
    return '\n'.join('    ' + s for s in stmts)


VARIANTS = {
    'K1': j(E_ST, E_CH, E_S1, CALL),
    'K2': j(E_ST, E_S0, E_CU, E_S1, CALL),
    'K3': j(E_ST, E_S1, E_CH, CALL),
    'K4': j(E_S1, E_ST, E_CH, CALL),
    'K5': j(E_CH, E_S1, E_ST, CALL),
    'K6': j(E_ST, E_CH, E_S1, "    fn_80045A10(this, param, mSpeed[1], (w >> 16) & 3, 1, 0, 0);"),
    'K7': j(E_ST, E_S0, E_S1, E_CU, CALL),
    'K8': j(E_ST, E_S1, E_S0, E_CU, CALL),
}

VARIANTS = {k: INCLUDE_LINE + '\n\n' + INIT_TPL.replace('@@BODY@@', v.rstrip()) + '\n' + EXECUTE_SRC
            for k, v in VARIANTS.items()}


def score(name):
    path = os.path.join(BASE, 'k_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='k_%s' % name)
    if not txt:
        print('%-4s COMPILE FAIL' % name)
        return False
    ok_i, msg_i = B.harness.diff_fn(B.TARGET, txt, B.FNS[0])
    ok_e, _ = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    ni = sum(1 for ln in msg_i.splitlines() if ' want: ' in ln)
    print('%-4s init:%s(%d) exec:%s%s' % (
        name, 'MATCH' if ok_i else 'diff ', ni, 'MATCH' if ok_e else 'diff',
        '  <<< WINNER' if (ok_i and ok_e) else ''))
    return ok_i and ok_e


if __name__ == '__main__':
    winner = None
    for n in sorted(VARIANTS):
        if score(n):
            winner = n
            break
    print('WINNER:', winner)
