import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round17')
txt = open(os.path.join(BASE, 'draft_disasm.txt'), encoding='utf-8', errors='replace').read()

print('bl __register_global_object:', txt.count('bl __register_global_object'))
print('__construct_array:', txt.count('__construct_array'))
print('__destroy_arr:', txt.count('__destroy_arr'))
print('arraydtor:', txt.count('arraydtor'))
print('BgObjName_tFv (dtor):', txt.count('BgObjName_tFv'))

# section info
p = os.popen(r'"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\bin\dtk-windows-x86_64.exe" elf info "%s"' % os.path.join(BASE, 'd_bg_actor_mng.o'))
print('\n' + p.read())
