"""Run the ORIGINAL poolcheck main-loop logic on a prebuilt object+disasm."""
import os, sys
sys.path.insert(0, os.path.join('tools','auto_decomp'))
sys.path.insert(0, os.path.join('scratch','poolcheck_rel'))
import harness
import poolcheck_orig as PC

obj, txt = sys.argv[1], sys.argv[2]
targets = sys.argv[3:]
draft = PC.parse_fns(txt)
target = {}
for p in targets: target.update(PC.parse_fns(p))
dpool, dol = PC.object_pool(obj), PC.pool.load()
pairs = []
for tname in target:
    if tname in draft: pairs.append((tname, tname)); continue
    cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
    if cand: pairs.append((tname, cand))
checked = mismatched = unresolved = 0
findings = []
for tname, dname in pairs:
    t, d = target[tname], draft[dname]
    if len(t) != len(d): continue
    gate = ([b for b,_ in t] == [b for b,_ in d]
            or harness.canonicalise([x for _,x in t]) == harness.canonicalise([x for _,x in d]))
    if not gate: continue
    for i, ((_, tt), (_, dt)) in enumerate(zip(t, d)):
        tm, dm = PC.POOL_REF.match(tt), PC.POOL_REF.match(dt)
        if tm and dm and tm.group(1) == dm.group(1):
            width = 4 if tm.group(1)=='lfs' else 8
            _, tv = PC.retail_value(tm.group(2), width, dol)
            raw = dpool.get(dm.group(2))
            if tv is None or (raw and PC.decode(raw,width)) is None and not raw: unresolved += 1
            else: checked += 1
    for i, va, tv, dv in PC.compare_pools(t, d, dpool, dol):
        mismatched += 1
        findings.append((tname,i,va,tv,dv,gate))
for name,i,va,tv,dv,gm in findings:
    print(('FALSE POSITIVE' if gm else 'differing fn')+': '+name)
    print(f'    instruction {i}: retail 0x{va:08X} = {tv!r}   draft = {dv!r}')
print(f'\n{checked} pooled constants compared by VALUE across {len(pairs)} paired functions')
print(f'{mismatched} mismatched, {unresolved} could not be resolved on one side')
