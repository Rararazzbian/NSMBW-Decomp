"""Disassemble the split objects covering d_a_wm_ghost's claimed ranges."""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
OBJDIR = os.path.join(ROOT, 'bin', 'dtkspl', 'd_basesNP', 'obj')

TARGETS = [
    ('auto_00_00163620_text.o', 't_163620.txt'),
    ('auto_fn_2_164180_text.o', 't_164180.txt'),
    ('auto_00_00164204_text.o', 't_164204.txt'),
    ('auto_03_00008880_rodata.o', 't_rodata_8880.txt'),
    ('auto_04_00044A68_data.o', 't_data_44A68.txt'),
    ('auto_05_0000FDC0_bss.o', 't_bss_FDC0.txt'),
]

for src, dst in TARGETS:
    obj = os.path.join(OBJDIR, src)
    out = os.path.join(HERE, dst)
    if os.path.exists(out) and os.path.getsize(out) > 0:
        print('skip (exists):', dst)
        continue
    ok, log = H.disasm(obj, out)
    print(('ok  ' if ok else 'FAIL'), src, '->', dst, log.strip()[:200])
