"""Reproduce the REL no-op with the ORIGINAL poolcheck.py logic, using the
already-built, verified course_r2 draft object + disassembly so that compiler
flags cannot confound the result."""
import os, sys, importlib.util
ROOT = os.path.abspath('.')
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
spec = importlib.util.spec_from_file_location(
    'poolcheck_orig', os.path.join(ROOT, 'scratch', 'poolcheck_rel', 'poolcheck_orig.py'))
PC = importlib.util.module_from_spec(spec); spec.loader.exec_module(PC)
import harness

W = os.path.join(ROOT, 'wip', 'course_r2')
TARGETS = ['auto_00_001604A0_text.o.txt', 'auto_fn_2_161890_text.o.txt',
           'auto_00_00161914_text.o.txt']
target = {}
for f in TARGETS:
    target.update(PC.parse_fns(os.path.join(W, f)))
draft = PC.parse_fns(os.path.join(W, 'draft.txt'))
dpool, dol = PC.object_pool(os.path.join(W, 'draft.o')), PC.pool.load()

pairs = []
for tname in target:
    if tname in draft:
        pairs.append((tname, tname)); continue
    cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
    if cand: pairs.append((tname, cand))

checked = mismatched = unresolved = 0
for tname, dname in pairs:
    t, d = target[tname], draft[dname]
    if len(t) != len(d): continue
    for i, ((_, tt), (_, dt)) in enumerate(zip(t, d)):
        tm, dm = PC.POOL_REF.match(tt), PC.POOL_REF.match(dt)
        if tm and dm and tm.group(1) == dm.group(1):
            checked += 1
print(f'{checked} pooled constants compared by VALUE across {len(pairs)} paired functions')
print(f'{mismatched} mismatched, {unresolved} could not be resolved on one side')

# now show the unit DOES contain pooled float constants
import re
nlfs = sum(1 for v in target.values() for _, tx in v if re.match(r'lfs|lfd', tx))
nsda = sum(1 for v in target.values() for _, tx in v if '@sda2' in tx)
print(f'\n... but the target contains {nlfs} lfs/lfd instructions and {nsda} @sda2 references.')
nlfs_d = sum(1 for v in draft.values() for _, tx in v if re.match(r'lfs|lfd', tx))
nsda_d = sum(1 for v in draft.values() for _, tx in v if '@sda2' in tx)
print(f'... the DRAFT contains {nlfs_d} lfs/lfd instructions and {nsda_d} @sda2 references.')
