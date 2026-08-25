"""Round 10: init sweep — nibbles extracted upfront as int locals."""
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

INIT_TPL = 'void daZoomPipeBase_c::init(u32 param) {\n@@BODY@@\n}\n'

CALL = "fn_80045A10(this, param, mSpeed[1], (float) d, 1, 0, 0);"


def j(*stmts):
    return '\n'.join('    ' + s for s in stmts)


PRE_U32 = [
    "    u32 w = mParam;",
    "    u32 a = (w >> 8) & 0xF;",
    "    u32 b = (w >> 4) & 0xF;",
    "    u32 c = w & 0xF;",
    "    u32 d = (w >> 16) & 3;",
]

VARIANTS = {
    # float stmt order: spd0, cur, spd1, step
    'N1': j(*(PRE_U32 + [
        "mSpeed[0] = b * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mSpeed[1] = c * 16.0f + 16.0f;",
        "mStep = a * 0.5f + 0.5f;",
        CALL])),
    # float stmt order: spd1, spd0, cur, step
    'N2': j(*(PRE_U32 + [
        "mSpeed[1] = c * 16.0f + 16.0f;",
        "mSpeed[0] = b * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mStep = a * 0.5f + 0.5f;",
        CALL])),
    # K2 order (step, spd0, cur, spd1) with int locals
    'N3': j(*(PRE_U32 + [
        "mStep = a * 0.5f + 0.5f;",
        "mSpeed[0] = b * 16.0f + 16.0f;",
        "mCurrent = mSpeed[0];",
        "mSpeed[1] = c * 16.0f + 16.0f;",
        CALL])),
    # chained current
    'N4': j(*(PRE_U32 + [
        "mStep = a * 0.5f + 0.5f;",
        "mSpeed[0] = mCurrent = b * 16.0f + 16.0f;",
        "mSpeed[1] = c * 16.0f + 16.0f;",
        CALL])),
    # int locals, float locals for speeds
    'N5': j(*([
        "    u32 w = mParam;",
        "    u32 a = (w >> 8) & 0xF;",
        "    u32 b = (w >> 4) & 0xF;",
        "    u32 c = w & 0xF;",
        "    u32 d = (w >> 16) & 3;",
        "    float v0 = b * 16.0f + 16.0f;",
        "    float v1 = c * 16.0f + 16.0f;",
        "    mStep = a * 0.5f + 0.5f;",
        "    mSpeed[0] = v0;",
        "    mCurrent = v0;",
        "    mSpeed[1] = v1;",
        CALL])),
}

VARIANTS = {k: INCLUDE_LINE + '\n\n' + INIT_TPL.replace('@@BODY@@', v.rstrip()) + '\n' + EXECUTE_SRC
            for k, v in VARIANTS.items()}


def score(name):
    path = os.path.join(BASE, 'n_%s.cpp' % name)
    with open(path, 'w') as fh:
        fh.write(VARIANTS[name])
    txt = B.build(path, label='n_%s' % name)
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
