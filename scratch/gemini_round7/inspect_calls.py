import re

with open('scratch/gemini_round7/auto_03_8016F330_text.o.txt') as f:
    d1 = f.read()
with open('scratch/gemini_round7/auto_03_8016F808_text.o.txt') as f:
    d2 = f.read()

full_text = d1 + '\n' + d2

fn_pat = re.compile(r'\.fn\s+(\S+),\s*(\S+)')
fns = []
current_fn = None
curr_data = {'calls': [], 'globals': [], 'lines': []}

for line in full_text.splitlines():
    m = fn_pat.match(line.strip())
    if m:
        if current_fn:
            fns.append((current_fn, curr_data))
        current_fn = m.group(1)
        curr_data = {'scope': m.group(2), 'calls': [], 'globals': [], 'lines': []}
    elif current_fn:
        curr_data['lines'].append(line)
        bm = re.search(r'\bbl\s+([^\s,]+)', line)
        if bm:
            curr_data['calls'].append(bm.group(1))
        gm = re.search(r'([A-Za-z0-9_@\$]+)@(sda21|sda|ha|l|toc)', line)
        if gm:
            curr_data['globals'].append(gm.group(0))

if current_fn:
    fns.append((current_fn, curr_data))

print(f'Parsed {len(fns)} functions.')
for idx, (fn_name, data) in enumerate(fns):
    calls_str = ', '.join(sorted(set(data['calls']))) if data['calls'] else 'none'
    globs_str = ', '.join(sorted(set(data['globals']))) if data['globals'] else 'none'
    scope = data['scope']
    print(f'FN {idx+1:2d} [{scope:6}]: {fn_name}')
    print(f'    Calls:   {calls_str}')
    print(f'    Globals: {globs_str}')
