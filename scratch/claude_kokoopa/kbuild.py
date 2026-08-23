import os, re, subprocess, sys
ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'gemini_round24')
INC = os.path.join(BASE, 'include')
GAME = os.path.join(BASE, 'game')
SP = r'C:\Users\Razz\AppData\Local\Temp\claude\C--Users-Razz-Documents-Projects-NSMBW-Decomp\5b140d7b-dd98-48ee-9746-feef21c1524b\scratchpad'
TARGET = os.path.join(SP, 'kok_target.txt')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

WORK = os.path.join(SP, 'kok')
os.makedirs(WORK, exist_ok=True)


def build(src_text, label):
    src = os.path.join(WORK, label + '.cpp')
    with open(src, 'w', encoding='utf-8') as fh:
        fh.write(src_text)
    obj = os.path.join(WORK, label + '.o')
    txt = os.path.join(WORK, label + '.txt')
    ok, log = harness.compile_draft(src, obj, extra_inc=(INC, GAME, BASE),
                                    module='wiimj2d')
    if not ok:
        return None, log
    ok, log = harness.disasm(obj, txt)
    if not ok:
        return None, log
    return txt, None


def score(txt, fn):
    r = subprocess.run([sys.executable,
                        os.path.join(ROOT, 'tools', 'auto_decomp', 'fndiff.py'),
                        TARGET, txt, fn], capture_output=True, text=True)
    return r.stdout


if __name__ == '__main__':
    base = open(os.path.join(BASE, 'd_enemy_toride_kokoopa.cpp'),
                encoding='utf-8').read()
    txt, err = build(base, 'base')
    if txt is None:
        print('COMPILE FAIL'); print(err[-3000:]); sys.exit(1)
    print(score(txt, 'initializeState_Jump__18dEnTorideKokoopa_cFv'))
    print(score(txt, 'initializeState_BigJump__18dEnTorideKokoopa_cFv'))
