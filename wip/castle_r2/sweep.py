"""Variant sweep for getKoopaShipStopPos.

Each variant is a full replacement of the function BODY (between the '{' line and
the closing '}').  Compiles, disassembles, reports:
  * differing count for fn_2_15FAA0
  * total unit score (to catch regressions elsewhere)
"""
import sys, os, re, subprocess, importlib.util

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
HERE = os.path.join(ROOT, 'wip', 'castle_r2')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

spec = importlib.util.spec_from_file_location("va", os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'))
va = importlib.util.module_from_spec(spec)
spec.loader.exec_module(va)

BASE_SRC = os.path.join(HERE, 'd_a_wm_castle.cpp')
SIG = 'mVec3_c daWmCastle_c::getKoopaShipStopPos() const {'

TARGET_OBJS = [
    os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015ECC0_text.o'),
    os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_fn_2_15FAE0_text.o'),
    os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj', 'auto_00_0015FBB4_text.o'),
]

def target_fn(addr):
    cache = os.path.join(ROOT, 'wip', 'wm_units', '_dis')
    os.makedirs(cache, exist_ok=True)
    for obj in TARGET_OBJS:
        out = os.path.join(cache, os.path.basename(obj) + '.txt')
        if not os.path.exists(out) or os.path.getsize(out) == 0:
            va.H.disasm(obj, out)
        for a, name, ins in va.functions(out, with_addr=True):
            if a == addr:
                return name, ins
    return None

def splice(body):
    txt = open(BASE_SRC, encoding='utf-8', newline='').read()
    i = txt.index(SIG)
    j = txt.index('\n}\n', i)
    return txt[:i] + SIG + '\n' + body.rstrip('\n') + txt[j:]

def run(tag, body, keep=False):
    src = os.path.join(HERE, 'v_%s.cpp' % tag)
    # NOTE: filename is part of the object code for anon-namespace symbols; this unit
    # has none, and verify_anon pairs by content, so a probe name is safe here.
    open(src, 'w', encoding='utf-8', newline='\n').write(splice(body))
    obj = src[:-4] + '.o'
    txt = src[:-4] + '.txt'
    ok, log = H.compile_draft(src, obj, extra_inc=[os.path.join(HERE, 'include')], module='d_basesNP')
    if not ok:
        print('%-28s BUILD FAIL' % tag)
        print(log[-1500:])
        return None
    H.disasm(obj, txt)
    r = subprocess.run([sys.executable, os.path.join(ROOT, 'wip', 'wm_units', 'verify_anon.py'),
                        txt, '0x15ecc0', '0x15fbe0'] + TARGET_OBJS,
                       capture_output=True, text=True)
    out = r.stdout
    m = re.search(r'(\d+)/(\d+) byte-identical', out)
    score = m.group(0) if m else '?'
    ml = re.search(r'0x0015faa0\s+\S+\s+(\d+)\s+(MATCH|(\d+) differing)', out)
    fn = ml.group(2) if ml else '?'
    print('%-28s  fn=%-14s unit=%s' % (tag, fn, score))
    if 'MATCH' not in (ml.group(2) if ml else ''):
        pass
    if not keep:
        for p in (obj,):
            pass
    return out, txt

if __name__ == '__main__':
    pass
