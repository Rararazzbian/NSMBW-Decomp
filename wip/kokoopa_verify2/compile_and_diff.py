import sys, os
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
WIP = os.path.join(ROOT, 'wip', 'kokoopa_verify2')
SRC = os.path.join(WIP, 'd_enemy_toride_kokoopa.cpp')
OBJ = os.path.join(WIP, 'd_enemy_toride_kokoopa.o')
TXT = os.path.join(WIP, 'd_enemy_toride_kokoopa.txt')
EXTRA_INC = os.path.join(ROOT, 'scratch', 'gemini_round17', 'include')

ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[EXTRA_INC], module='wiimj2d')
print('COMPILE OK:', ok)
if not ok:
    print(log[-4000:])
    sys.exit(1)

dok, dlog = harness.disasm(OBJ, TXT)
print('DISASM OK:', dok)
if not dok:
    print(dlog[-2000:])
    sys.exit(1)

print('compile+disasm succeeded.')
