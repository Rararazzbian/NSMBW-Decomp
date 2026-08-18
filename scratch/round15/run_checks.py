"""Compile the ghost draft and run all three checks (verify_anon, check_sections --dump, check_vtable)."""
import os, sys, subprocess

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

SRC = os.path.join(ROOT, 'scratch/round15/d_a_wm_ghost.cpp')
OBJ = os.path.join(ROOT, 'scratch/round15/draft.o')
TXT = os.path.join(ROOT, 'scratch/round15/draft.txt')
INC = [os.path.join(ROOT, 'scratch/round15/include')]

OBJDIR = os.path.join(ROOT, 'bin/dtkspl/d_basesNP/obj')
TEXT_OBJS = [
    os.path.join(OBJDIR, 'auto_00_00163620_text.o'),
    os.path.join(OBJDIR, 'auto_fn_2_164180_text.o'),
    os.path.join(OBJDIR, 'auto_00_00164204_text.o'),
]
DATA_TXT = os.path.join(OBJDIR, 'auto_04_00044A68_data.txt')
VT_LABEL = 'lbl_2_data_44B78'

print('=== COMPILE ===')
ok, log = H.compile_draft(SRC, OBJ, extra_inc=INC, module='d_basesNP')
if not ok:
    print('COMPILE FAILED')
    print(log[-6000:])
    sys.exit(2)
print('compile ok')

print('\n=== DISASM ===')
H.disasm(OBJ, TXT)
print('disasm ok')

print('\n=== VERIFY_ANON  [0x163620, 0x164230) ===')
cmd = [sys.executable, os.path.join(ROOT, 'wip/wm_units/verify_anon.py'), TXT,
       '0x163620', '0x164230'] + TEXT_OBJS
subprocess.run(cmd, cwd=ROOT)

print('\n=== CHECK_SECTIONS --dump ===')
cmd = [sys.executable, os.path.join(ROOT, 'wip/wm_units/check_sections.py'), OBJ,
       'd_basesNP', '--dump']
subprocess.run(cmd, cwd=ROOT)

print('\n=== CHECK_VTABLE ===')
cmd = [sys.executable, os.path.join(ROOT, 'wip/wm_units/check_vtable.py'), TXT,
       DATA_TXT, VT_LABEL, '0x163620', '0x164230'] + TEXT_OBJS
subprocess.run(cmd, cwd=ROOT)
