import os
import re
import subprocess
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
OBJ = os.path.join(BASE, 'd_bg_actor_mng.o')

# raw disasm text
txt = open(os.path.join(BASE, 'draft_disasm.txt'), encoding='utf-8', errors='replace').read()

print('bl __register_global_object count:', txt.count('bl __register_global_object'))
print('references to __register_global_object:', txt.count('__register_global_object'))
print('references to __destroy_arr:', txt.count('__destroy_arr'))
print('references to __construct_array:', txt.count('__construct_array'))
print('arraydtor mentions:', txt.count('arraydtor'))
print('__dt__Q217dBgActorManager_c11BgObjName_tFv mentions:', txt.count('BgObjName_tFv'))

# Now dump the raw symbol table of the .o to see ALL symbols (dtk may hide local ones)
sym = os.path.join(BASE, 'draft_symbols.txt')
p = subprocess.run([harness.DTK, 'elf', 'symbols', OBJ, sym], cwd=ROOT, capture_output=True, text=True)
print('\nsymbols dump rc:', p.returncode)
if p.returncode == 0:
    s = open(sym, encoding='utf-8', errors='replace').read()
    for line in s.splitlines():
        low = line.lower()
        if any(k in low for k in ('arraydtor', 'bgobjname', 'register', 'bss', 'ctors', 'sinit', 'l_object', 'l_pa3')):
            print(line)
