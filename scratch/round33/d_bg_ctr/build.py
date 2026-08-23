import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'round33', 'd_bg_ctr')
SHADOW = os.path.join(BASE, 'shadow')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

TARGET = os.path.join(BASE, 'target.txt')

def build(source, label, function):
    src = source if os.path.isabs(source) else os.path.join(BASE, source)
    obj = os.path.join(BASE, label + '.o')
    txt = os.path.join(BASE, label + '.txt')
    ok, log = harness.compile_draft(src, obj, extra_inc=(SHADOW,), module='wiimj2d')
    if not ok:
        raise RuntimeError('compile failed for %s:\n%s' % (label, log))
    ok, log = harness.disasm(obj, txt)
    if not ok:
        raise RuntimeError('disassembly failed for %s:\n%s' % (label, log))
    body = harness.extract(txt, function)
    if body is None:
        raise RuntimeError('function %s missing from %s' % (function, txt))
    words = len(body)
    with open(txt, encoding='utf-8', errors='replace') as fh:
        lines = fh.readlines()
    start = None
    end = None
    want = harness.norm_name(function)
    for i, line in enumerate(lines):
        match = harness.FN_START.match(line.strip())
        if match and harness.norm_name(match.group(1)) == want:
            start = i
            break
    if start is None:
        raise RuntimeError('function %s metadata missing from %s' % (function, txt))
    for i in range(start + 1, len(lines)):
        if harness.FN_END.match(lines[i].strip()):
            end = i
            break
    section = ''.join(lines[start:end])
    frame_match = re.search(r'stwu\s+r1,\s+-0x([0-9A-Fa-f]+)\(r1\)', section)
    frame = ('0x%X' % int(frame_match.group(1), 16)) if frame_match else None
    gpr = re.findall(r'\b(?:bl|b)\s+_savegpr_(\d+)\b', section)
    fpr = re.findall(r'\b(?:bl|b)\s+_savefpr_(\d+)\b', section)
    if not fpr:
        fpr = [int(x) for x in re.findall(r'\bstfd\s+f(\d+),', section)
               if int(x) >= 14]
    if not gpr:
        gpr = [int(x) for x in re.findall(r'\bstw\s+r(\d+),', section)
               if int(x) >= 14]
    return words, frame, sorted(set(map(int, gpr))), sorted(set(map(int, fpr))), obj, txt

if __name__ == '__main__':
    print(build(os.path.join(ROOT, 'scratch', 'round27', 'd_bg_ctr', 'calc_v1_decl_order.cpp'),
                'baseline_calc', 'calc__9dBg_ctr_cFv'))
