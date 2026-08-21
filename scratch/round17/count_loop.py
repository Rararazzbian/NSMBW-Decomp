import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

# Read both disassemblies
with open(TARGET) as f:
    target_lines = f.readlines()
with open(DIS) as f:
    draft_lines = f.readlines()

# Find ProcMain in both
def find_fn(lines, name):
    start = None
    for i, line in enumerate(lines):
        if f'.fn {name}' in line:
            start = i
        if start is not None and '.endfn' in line:
            return start, i
    return None, None

ts, te = find_fn(target_lines, 'ProcMain__17dBgActorManager_cFv')
ds, de = find_fn(draft_lines, 'ProcMain__17dBgActorManager_cFv')

target_body = [l for l in target_lines[ts:te+1] if l.strip() and not l.strip().startswith('.') and not l.strip().startswith('#')]
draft_body = [l for l in draft_lines[ds:de+1] if l.strip() and not l.strip().startswith('.') and not l.strip().startswith('#')]

print(f'Target ProcMain: {len(target_body)} instructions')
print(f'Draft ProcMain: {len(draft_body)} instructions')
print(f'Difference: {len(target_body) - len(draft_body)}')

# Count instructions in the loop body (between the branch at the end of preamble and the epilogue)
# Target loop starts at .L_8007E5DC and ends at .L_8007E7BC
# Draft loop starts at .L_0000059C and ends at .L_00000758

# Find the loop body in target
t_loop_start = None
t_loop_end = None
for i, line in enumerate(target_lines[ts:te+1]):
    if 'L_8007E5DC' in line:
        t_loop_start = i
    if 'L_8007E7BC' in line:
        t_loop_end = i
        break

# Find the loop body in draft
d_loop_start = None
d_loop_end = None
for i, line in enumerate(draft_lines[ds:de+1]):
    if 'L_0000059C' in line:
        d_loop_start = i
    if 'L_00000758' in line:
        d_loop_end = i
        break

if t_loop_start is not None and t_loop_end is not None:
    t_loop_body = [l for l in target_lines[ts+t_loop_start:ts+t_loop_end+1] if l.strip() and not l.strip().startswith('.') and not l.strip().startswith('#')]
    print(f'\nTarget loop body: {len(t_loop_body)} instructions')
else:
    print(f'\nCould not find target loop bounds: start={t_loop_start}, end={t_loop_end}')

if d_loop_start is not None and d_loop_end is not None:
    d_loop_body = [l for l in draft_lines[ds+d_loop_start:ds+d_loop_end+1] if l.strip() and not l.strip().startswith('.') and not l.strip().startswith('#')]
    print(f'Draft loop body: {len(d_loop_body)} instructions')
else:
    print(f'\nCould not find draft loop bounds: start={d_loop_start}, end={d_loop_end}')
