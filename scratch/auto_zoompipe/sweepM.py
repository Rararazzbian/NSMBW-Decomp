"""Round 9: init sweep — call-arg duplication of the speed expression."""
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

INIT_TPL = 'void daZoomPipeBase_c::init(u32 param) {\n    u32 w = mParam;\n\n@@BODY@@\n}\n'

SPD1 = "(w & 0xF) * 16.0f + 16.0f"


def j(*stmts):
    return '\n'.join('    ' + s for s in stmts)


CALL_DUP_CAST = "fn_80045A10(this, param, %s, (float) ((w >> 16) & 3), 1, 0, 0);" % SPD1
CALL_DUP_PLAIN = "fn_80045A10(this, param, %s, (w >> 16) & 3, 1, 0, 0);" % SPD1
CALL_MEM = "fn_80045A10(this, param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);"

VARIANTS = {
    'M1': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mSpeed[1] = %s;" % SPD1,
        CALL_DUP_CAST),
    'M2': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[0] = mCurrent = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mSpeed[1] = %s;" % SPD1,
        CALL_DUP_CAST),
    'M3': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mCurrent = mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mSpeed[1] = %s;" % SPD1,
        CALL_DUP_CAST),
    'M4': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mSpeed[1] = %s;" % SPD1,
        CALL_DUP_PLAIN),
    'M5': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mSpeed[1] = %s;" % SPD1,
        CALL_MEM),
    'M6': j(
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[1] = %s;" % SPD1,
        "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        CALL_DUP_CAST),
    'M7': j(
        "mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;",
        "mSpeed[1] = %s;" % SPD1,
        CALL_DUP_CAST),
}

VARIANTS = {k: INCLUDE_LINE + '\n\n' + INIT_TPL.replace('@@BODY@@', v.rstrip()) + '\n' + EXECUTE_SRC
            for k, v in VARIANTS.items()}


def score(name):
    path = os.path.join(BASE, 'm_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='m_%s' % name)
    if not txt:
        print('%-4s COMPILE FAIL' % name)
        return None
    ok_i, msg_i = B.harness.diff_fn(B.TARGET, txt, B.FNS[0])
    ok_e, _ = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    ni = sum(1 for ln in msg_i.splitlines() if ' want: ' in ln)
    print('%-4s init:%s(%d) exec:%s%s' % (
        name, 'MATCH' if ok_i else 'diff ', ni, 'MATCH' if ok_e else 'diff',
        '  <<< WINNER' if (ok_i and ok_e) else ''))
    return ni


if __name__ == '__main__':
    best = None
    for n in sorted(VARIANTS):
        ni = score(n)
        if ni == 0:
            best = n
            break
        if ni is not None and (best is None or ni < best[1]):
            best = (n, ni)
    print('BEST:', best)
