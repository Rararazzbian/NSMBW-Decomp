"""Variant build/score driver for KokoopaSpFumiCheck_c::operate register search."""
import os
import re
import sys

ROOT = '/opt/NSMBW-Decomp'
BASE = '/opt/NSMBW-Decomp/scratch/auto_kokoopa_fumi'
HERE = os.path.join(BASE, 'sw_perm')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

harness.MWCC = os.path.join(BASE, 'shims', 'mwcceppc')
harness.DTK = os.path.join(BASE, 'dtk')
TARGET = os.path.join(ROOT, 'tools', 'auto_decomp', 'work',
                      'dol_bases_d_kokoopa_sp_fumi_check', 'target.txt')
NAME = 'operate__20KokoopaSpFumiCheck_cFRiP5dEn_cR12FumiCcInfo_c'


def build(src, label):
    """src absolute path; outputs sw_perm/<label>.o/.txt. Returns txt path or None+log."""
    obj = f'{HERE}/{label}.o'
    txt = f'{HERE}/{label}.txt'
    ok, log = harness.compile_draft(src, obj,
                                    extra_inc=(f'{BASE}/shadow',), module='wiimj2d')
    if not ok:
        return False, log
    ok, log = harness.disasm(obj, txt)
    return ok, (txt if ok else log)


def score(txt):
    matched, msg = harness.diff_fn(TARGET, txt, NAME)
    t = harness.extract(TARGET, NAME)
    d = harness.extract(txt, NAME) or []
    return matched, len(t), len(d), (msg or '')


def homes(txt):
    """Report which callee-saved reg holds out / pl / en by reading the prologue
    and one late store of each role."""
    insns = harness.extract(txt, NAME) or []
    body = '\n'.join(insns)
    out_reg = pl_reg = en_reg = '?'

    # mr rXX, r4  -> out home ; mr rXX, r5 -> en home
    m = re.search(r'\bmr\s+(r\d+),\s*r4\b', body)
    if m:
        out_reg = m.group(1)
    m = re.search(r'\bmr\s+(r\d+),\s*r5\b', body)
    if m:
        en_reg = m.group(1)

    # pl: lwz rA, 0x8(r6); lwz rPL, 0x4(rA)
    m = re.search(r'lwz\s+r(\d+),\s*0x8\(r6\)\s*;?\s*\n.*?lwz\s+r(\d+),\s*0x4\(r\1\)',
                  body, re.S)
    if m:
        pl_reg = 'r' + m.group(2)

    return f'out={out_reg} pl={pl_reg} en={en_reg}'


def run(srcname):
    src = os.path.join(HERE, srcname)
    label = os.path.splitext(srcname)[0]
    ok, log = build(src, label)
    if not ok:
        print(f'{label}: COMPILE FAIL\n{log}')
        return False
    txt = log  # build() returns the txt path on success
    matched, tw, dw, msg = score(txt)
    status = 'MATCH' if matched else (msg.splitlines()[0] if msg else 'DIFFS?')
    print(f'{label}: {"MATCH" if matched else "diff"} ({tw}w/{dw}w) {status} | {homes(txt)}')
    return matched


if __name__ == '__main__':
    for name in sys.argv[1:]:
        run(name)
