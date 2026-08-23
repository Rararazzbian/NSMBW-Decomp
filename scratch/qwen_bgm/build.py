"""Compile a draft in this directory, disassemble it, and report the shape.

    python -c "import sys; sys.path.insert(0,'scratch/qwen_bgm'); import build; \
               print(build.build('NAME.cpp','NAME','execute__13dIceEfMaker_cFv'))"
"""
import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
BASE = os.path.join(ROOT, 'scratch', 'qwen_bgm')
SHADOW = os.path.join(BASE, 'shadow')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

TARGET = os.path.join(BASE, 'target.txt')


def build(source, label, function):
    src = source if os.path.isabs(source) else os.path.join(BASE, source)
    obj = os.path.join(BASE, label + '.o')
    txt = os.path.join(BASE, label + '.txt')
    inc = (SHADOW,) if os.path.isdir(SHADOW) else ()
    ok, log = harness.compile_draft(src, obj, extra_inc=inc, module='wiimj2d')
    if not ok:
        raise RuntimeError('compile failed for %s:\n%s' % (label, log))
    ok, log = harness.disasm(obj, txt)
    if not ok:
        raise RuntimeError('disassembly failed for %s:\n%s' % (label, log))
    body = harness.extract(txt, function)
    if body is None:
        raise RuntimeError('function %s missing from %s' % (function, txt))
    section = '\n'.join(body)
    m = re.search(r'stwu\s+r1,\s*-0x([0-9A-Fa-f]+)\(r1\)', section)
    frame = ('0x%X' % int(m.group(1), 16)) if m else None
    gpr = sorted({int(x) for x in re.findall(r'\bstw\s+r(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', section)
                  if int(x) >= 13})
    fpr = sorted({int(x) for x in re.findall(r'\bstfd\s+f(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', section)
                  if int(x) >= 14})
    return len(body), frame, gpr, fpr, obj, txt
