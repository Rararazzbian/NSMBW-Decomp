import io
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, errors='replace')

txt = open(r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\trial\corpus_line.txt',
           encoding='utf-8', errors='replace').read()
fns = re.split(r'(?m)^\.fn ', txt)
INSN_RE = re.compile(r'\*/\s*(\S.*)$')

for f in fns[1:]:
    name = f.split(',')[0]
    if not ('sda21' in f and re.search(r'lfs f1, 0x', f)):
        continue
    lines = []
    for line in f.splitlines()[1:]:
        m = INSN_RE.search(line)
        if m:
            lines.append(m.group(1).strip())
        if len(lines) == 10:
            break
    # keep only functions whose 4th-6th insns involve fmuls/fadds/fsubs near a pool load
    blob = ' | '.join(lines)
    if 'fmuls' in blob or 'fadds' in blob or 'fsubs' in blob:
        print('==', name)
        for i, l in enumerate(lines):
            print('  %2d %s' % (i, l))
