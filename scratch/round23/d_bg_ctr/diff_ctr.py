import os
import re
import sys

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

BASE = os.path.join(ROOT, 'scratch', 'round23', 'd_bg_ctr')
DIS = os.path.join(BASE, 'draft_disasm.txt')
TARGET = os.path.join(BASE, 'target.txt')

# All real functions in target order (skip gap_/pad_ entries)
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
names = []
with open(TARGET, encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = FN_START.match(line.strip())
        if m and not m.group(1).startswith('gap_') and not m.group(1).startswith('pad_'):
            names.append(m.group(1).strip().strip('"'))

# draft function names (to map fn_8007xxxx targets to mangled draft names)
draft_names = []
with open(DIS, encoding='utf-8', errors='replace') as fh:
    for line in fh:
        m = FN_START.match(line.strip())
        if m:
            draft_names.append(m.group(1).strip().strip('"'))

def resolve(target_name):
    """map a target fn_8007XXXX name onto the draft's mangled equivalent"""
    if target_name.startswith('fn_') and target_name not in draft_names:
        for d in draft_names:
            if d.startswith(target_name + '__'):
                return d
    return target_name

def cmp_bodies(want, got, label):
    """compare two extracted instruction lists, print report"""
    if want is None:
        return 'TARGET MISSING'
    if got is None:
        return 'DRAFT MISSING'
    if want == got:
        return 'MATCHING (%d instructions)' % len(want)
    lines = ['size: target %d, draft %d' % (len(want), len(got))]
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else '<none>'
        b = got[i] if i < len(got) else '<none>'
        if a != b:
            lines.append('  %3d | want: %-48s got: %s' % (i, a, b))
        if len(lines) > 14:
            lines.append('  ... truncated')
            break
    return '\n'.join(lines)

print('target functions: %d' % len(names))
matched = 0
differ = 0
missing = 0
for name in names:
    draft_name = resolve(name)
    if draft_name != name:
        # different names on each side: compare bodies directly
        report = cmp_bodies(harness.extract(TARGET, name),
                            harness.extract(DIS, draft_name), name)
        if report.startswith('MATCHING'):
            matched += 1
            print('MATCH  %-70s %s' % (name, report))
            continue
        if report.startswith('DRAFT MISSING') or report.startswith('TARGET MISSING'):
            missing += 1
            print('MISS   %s (%s)' % (name, report))
            continue
        differ += 1
        print('DIFF   %-70s %s' % (name, report.splitlines()[0]))
        for l in report.splitlines()[1:8]:
            print('        %s' % l)
        continue
    ok, report = harness.diff_fn(TARGET, DIS, name)
    lines = report.splitlines()
    if ok:
        matched += 1
        sz = lines[0] if lines else ''
        print('MATCH  %-70s %s' % (name, sz))
    else:
        if lines and lines[0].startswith('DRAFT MISSING'):
            missing += 1
            print('MISS   %s' % name)
            continue
        differ += 1
        size = lines[0] if lines else '?'
        print('DIFF   %-70s %s' % (name, size))
        for l in lines[1:8]:
            print('        %s' % l)
print()
print('MATCHED: %d  DIFFER: %d  MISSING: %d  (target %d)' % (matched, differ, missing, len(names)))
