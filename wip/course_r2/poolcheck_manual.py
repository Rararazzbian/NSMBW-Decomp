"""Manual poolcheck pairing for d_a_wm_course.cpp, since its target functions
are all unnamed fn_2_* labels and poolcheck.py's automatic pairing (exact name
match, or the static-suffix rule) cannot pair any of them to the draft's real
names. Pairs are taken from the verified build.py / verify_anon.py mapping.
"""
import os, sys
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import harness as H
sys.path.insert(0, os.path.join('tools', 'auto_decomp'))
import poolcheck as PC

HERE = os.path.dirname(os.path.abspath(__file__))
DRAFT_TXT = os.path.join(HERE, 'draft.txt')
DRAFT_OBJ = os.path.join(HERE, 'draft.o')

TARGET_FILES = [
    os.path.join(HERE, 'auto_00_001604A0_text.o.txt'),
    os.path.join(HERE, 'auto_fn_2_161890_text.o.txt'),
    os.path.join(HERE, 'auto_00_00161914_text.o.txt'),
]

PAIRS = [
    ('fn_2_1604A0', 'daWmCourse_c_classInit__Fv'),
    ('fn_2_1604D0', '__ct__12daWmCourse_cFv'),
    ('fn_2_160560', '__dt__12daWmCourse_cFv'),
    ('fn_2_160610', 'create__12daWmCourse_cFv'),
    ('fn_2_160990', 'execute__12daWmCourse_cFv'),
    ('fn_2_160A60', 'draw__12daWmCourse_cFv'),
    ('fn_2_160A90', 'doDelete__12daWmCourse_cFv'),
    ('fn_2_160AA0', 'createModel__12daWmCourse_cFv'),
    ('fn_2_160E50', 'calcModel__12daWmCourse_cFv'),
    ('fn_2_160F00', 'updateState__12daWmCourse_cFv'),
    ('fn_2_160F50', 'processCutsceneCommand__12daWmCourse_cFib'),
    ('fn_2_161170', 'setMatClrAnm__12daWmCourse_cFiff'),
    ('fn_2_161220', 'updateOpenAnim__12daWmCourse_cFv'),
    ('fn_2_161390', 'searchOpenNeighbor__12daWmCourse_cFv'),
    ('fn_2_161420', 'openNeighbors__12daWmCourse_cFb'),
    ('fn_2_161580', 'getMatClrFrame__12daWmCourse_cFv'),
    ('fn_2_161590', 'updateSpecialWorld__12daWmCourse_cFv'),
    ('fn_2_1615F0', 'updateClearAnim__12daWmCourse_cFb'),
    ('fn_2_161790', 'updateHelpFade__12daWmCourse_cFv'),
    ('fn_2_161840', 'isWorld2SpecialType__12daWmCourse_cFv'),
    ('fn_2_161870', 'vf78__12daWmCourse_cFv'),
    ('fn_2_161890', '__sinit_\\d_a_wm_course_cpp'),
    ('fn_2_161920', '__arraydtor$12812'),
]

# parse all three target files into one name->instrs dict
target = {}
for f in TARGET_FILES:
    target.update(PC.parse_fns(f))
draft = PC.parse_fns(DRAFT_TXT)

dpool = PC.object_pool(DRAFT_OBJ)
dol = PC.pool.load()

checked = mismatched = unresolved = 0
findings = []
for tname, dname in PAIRS:
    t = target.get(tname)
    d = draft.get(dname)
    if t is None:
        print('TARGET MISSING', tname); continue
    if d is None:
        print('DRAFT MISSING', dname); continue
    if len(t) != len(d):
        print(f'LEN MISMATCH {tname}/{dname}: target {len(t)} draft {len(d)}')
        continue
    gate_matched = ([b for b, _ in t] == [b for b, _ in d]
                    or H.canonicalise([x for _, x in t])
                    == H.canonicalise([x for _, x in d]))
    for i, ((_, ttext), (_, dtext)) in enumerate(zip(t, d)):
        tm, dm = PC.POOL_REF.match(ttext), PC.POOL_REF.match(dtext)
        if tm and dm and tm.group(1) == dm.group(1):
            width = 4 if tm.group(1) == 'lfs' else 8
            _, tv = PC.retail_value(tm.group(2), width, dol)
            raw = dpool.get(dm.group(2))
            if tv is None or (raw and PC.decode(raw, width)) is None and not raw:
                unresolved += 1
            else:
                checked += 1
    for i, va, tv, dv in PC.compare_pools(t, d, dpool, dol):
        mismatched += 1
        findings.append((tname, dname, i, va, tv, dv, gate_matched))

for tname, dname, i, va, tv, dv, gm in findings:
    flag = 'FALSE POSITIVE' if gm else 'differing fn'
    print(f'{flag}: {tname} <-> {dname}')
    print(f'    instruction {i}: retail 0x{va:08X} = {tv!r}   draft = {dv!r}')

print(f'\n{checked} pooled constants compared by VALUE across {len(PAIRS)} paired functions')
print(f'{mismatched} mismatched, {unresolved} could not be resolved on one side')
