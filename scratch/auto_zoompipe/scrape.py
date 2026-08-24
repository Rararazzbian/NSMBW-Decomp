"""Score every existing disassembly .txt in scratch/auto_zoompipe against target."""
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

rows = []
for fn in sorted(os.listdir(BASE)):
    if not fn.endswith('.txt') or fn == 'target.txt':
        continue
    label = fn[:-4]
    txt = os.path.join(BASE, fn)
    ok_i, msg_i = B.harness.diff_fn(B.TARGET, txt, B.FNS[0])
    ok_e, _ = B.harness.diff_fn(B.TARGET, txt, B.FNS[1])
    ni = sum(1 for ln in msg_i.splitlines() if ' want: ' in ln)
    rows.append((ni, label, 'MATCH' if ok_i else '', 'MATCH' if ok_e else ''))

rows.sort()
for ni, label, mi, me in rows:
    print('%-14s init:%-5s(%2d) exec:%s' % (label, mi or 'diff', ni, me))
