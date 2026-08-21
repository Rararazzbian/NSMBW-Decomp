import os, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

BASE = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target_8007E17C.txt')

with open(TARGET) as f:
    target_lines = f.readlines()
with open(DIS) as f:
    draft_lines = f.readlines()

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

# Get just the instruction lines (skip .fn, .endfn, comments, blank)
def get_instructions(lines, start, end):
    instrs = []
    for line in lines[start:end+1]:
        s = line.strip()
        if not s or s.startswith('#') or s.startswith('.fn') or s.startswith('.endfn'):
            continue
        instrs.append(s)
    return instrs

t_instrs = get_instructions(target_lines, ts, te)
d_instrs = get_instructions(draft_lines, ds, de)

print(f'Target instructions: {len(t_instrs)}')
print(f'Draft instructions: {len(d_instrs)}')
print(f'Difference: {len(t_instrs) - len(d_instrs)}')

# Find the loop body boundaries - search with colon suffix
t_loop_start = None
t_loop_end = None
for i, line in enumerate(t_instrs):
    if 'L_8007E5DC:' in line:
        t_loop_start = i
    if 'L_8007E7BC:' in line:
        t_loop_end = i
        break

d_loop_start = None
d_loop_end = None
for i, line in enumerate(d_instrs):
    if 'L_0000059C:' in line:
        d_loop_start = i
    if 'L_00000758:' in line:
        d_loop_end = i
        break

print(f'\nTarget loop: {t_loop_start} to {t_loop_end} ({t_loop_end - t_loop_start + 1} instrs)')
print(f'Draft loop: {d_loop_start} to {d_loop_end} ({d_loop_end - d_loop_start + 1} instrs)')

# Count preamble and epilogue
t_preamble = t_instrs[:t_loop_start]
t_loop_body = t_instrs[t_loop_start:t_loop_end+1]
t_epilogue = t_instrs[t_loop_end+1:]

d_preamble = d_instrs[:d_loop_start]
d_loop_body = d_instrs[d_loop_start:d_loop_end+1]
d_epilogue = d_instrs[d_loop_end+1:]

print(f'\nTarget preamble: {len(t_preamble)} instrs')
print(f'Target loop body: {len(t_loop_body)} instrs')
print(f'Target epilogue: {len(t_epilogue)} instrs')

print(f'\nDraft preamble: {len(d_preamble)} instrs')
print(f'Draft loop body: {len(d_loop_body)} instrs')
print(f'Draft epilogue: {len(d_epilogue)} instrs')

# Now let's look at the actual differences in the loop body
# Compare instruction by instruction
print('\n=== Loop body instruction comparison ===')
for i in range(max(len(t_loop_body), len(d_loop_body))):
    t = t_loop_body[i] if i < len(t_loop_body) else '(missing)'
    d = d_loop_body[i] if i < len(d_loop_body) else '(missing)'
    if t != d:
        print(f'  [{i}] T: {t}')
        print(f'  [{i}] D: {d}')
        print()
