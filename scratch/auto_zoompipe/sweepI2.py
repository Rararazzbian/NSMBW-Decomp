"""Round 7: init() shape x order sweep."""
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

ALL_SRC = open(os.path.join(BASE, 's_S_CST.cpp')).read()
INCLUDE_LINE = '#include <game/bases/d_a_zoom_pipe_base.hpp>'
EXECUTE_SRC = '\n\n' + ALL_SRC[ALL_SRC.index('int daZoomPipeBase_c::execute()'):].rstrip() + '\n'

INIT_TPL = 'void daZoomPipeBase_c::init(u32 param) {\n    u32 w = mParam;\n\n@@BODY@@\n}\n'

CALL = "    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);"

E_S0 = "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;"
E_S1 = "mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;"
E_ST = "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;"
E_CU = "mCurrent = mSpeed[0];"
E_CHAIN = "mCurrent = mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;"

def j(*stmts):
    return '\n\n'.join(list(stmts) + [CALL])

VARIANTS = {
    # chained assignment shapes
    'W1': INIT_TPL.replace('@@BODY@@', j(E_ST, E_CHAIN, E_S1)),
    'W2': INIT_TPL.replace('@@BODY@@', j(E_CHAIN, E_S1, E_ST)),
    'W3': INIT_TPL.replace('@@BODY@@', j(E_S1, E_CHAIN, E_ST)),
    'W4': INIT_TPL.replace('@@BODY@@', j(E_ST, E_S1, E_CHAIN)),
    # local intermediate for the shared speed value
    'W5': INIT_TPL.replace('@@BODY@@', j(
        "    float v = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        E_ST,
        "    mSpeed[0] = v;",
        E_S1,
        "    mCurrent = v;")),
    'W6': INIT_TPL.replace('@@BODY@@', j(
        "    float v = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "    mSpeed[0] = v;",
        E_S1,
        "    mCurrent = v;",
        E_ST)),
    # step last, plain statements
    'W7': INIT_TPL.replace('@@BODY@@', j(E_S0, E_S1, E_CU, E_ST)),
    # speeds via one combined line each, current chained, step between
    'W8': INIT_TPL.replace('@@BODY@@', j(
        "    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;",
        "    mCurrent = mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        E_ST)),
}


VARIANTS = {k: INCLUDE_LINE + '\n\n' + v.rstrip() + '\n' + EXECUTE_SRC for k, v in VARIANTS.items()}


def score(name):
    path = os.path.join(BASE, 'j_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='j_%s' % name)
    if not txt:
        print('%-4s COMPILE FAIL' % name)
        return False
    ok_i, msg_i = B.harness.diff_fn(B.TARGET, txt, B.FNS[0])
    ok_e, _ = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    ni = sum(1 for ln in msg_i.splitlines() if ' want: ' in ln)
    print('%-4s init:%s(%d) exec:%s%s' % (
        name, 'MATCH' if ok_i else 'diff', ni, 'MATCH' if ok_e else 'diff',
        '  <<< WINNER' if (ok_i and ok_e) else ''))
    return ok_i and ok_e


if __name__ == '__main__':
    winner = None
    for n in sorted(VARIANTS):
        if score(n):
            winner = n
            break
    print('WINNER:', winner)
