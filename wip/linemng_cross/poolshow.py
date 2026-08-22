"""Per-function pooled-constant listing for the nine cross-check functions.

`poolcheck.py` prints a unit-wide total; a per-function claim needs a
per-function receipt.  Prints, for each named function, every pooled load with
retail's decoded VALUE beside the draft's, so a match can be read directly
rather than inferred from an aggregate count of zero.

    python poolshow.py <draft.cpp> <shadow_include>
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'line_mng_shared'))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools', 'auto_decomp'))
import harness
import poolcheck
import tally

WANT = [
    'line_cross_chk1__',
    'line_cross_chk2__',
    'lineRHUL_cross_chk__',
    'lineRHUR_cross_chk__',
    'lineRHLL_cross_chk__',
    'circle_ul2_cross_chk__',
    'circle_ur2_cross_chk__',
    'circle_dl2_cross_chk__',
    'check_term__',
]


def main():
    src = os.path.abspath(sys.argv[1])
    inc = os.path.abspath(sys.argv[2])
    work = os.path.join(HERE, '_pool')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)
    d, t = tally.parse(txt), tally.parse(poolcheck.__dict__.get('TARGET') or
                                         os.path.join(HERE, '..', 'line_mng_shared', 'target.txt'))
    dpool, dol = poolcheck.object_pool(obj), poolcheck.pool.load()

    total = 0
    for w in WANT:
        dk = next((k for k in d if k.startswith(w)), None)
        tk = next((k for k in t if k.startswith(w)), None)
        if not dk or not tk:
            print(f'{w}: NOT FOUND draft={dk} target={tk}')
            continue
        bad = poolcheck.compare_pools(t[tk], d[dk], dpool, dol)
        rows = []
        for i, (tb, tt) in enumerate(t[tk]):
            m = poolcheck.POOL_REF.match(tt)
            if not m:
                continue
            width = 8 if m.group(1) == 'lfd' else 4
            tv = poolcheck.retail_value(m.group(2), width, dol)
            dm = poolcheck.POOL_REF.match(d[dk][i][1]) if i < len(d[dk]) else None
            dv = (poolcheck.decode(dpool.get(dm.group(2), b''), width)
                  if dm and dm.group(2) in dpool else None)
            rows.append(f'      w{i:<4d} {m.group(1)}  retail {tv!r:<22} draft {dv!r}')
        total += len(rows)
        status = 'OK' if not bad else f'{len(bad)} MISMATCH'
        print(f'  {tk[:60]}')
        print(f'      {len(t[tk])}w   {len(rows)} pooled constant(s)   {status}')
        print('\n'.join(rows) if rows else '      (no pooled loads)')
    print(f'\n{total} pooled constants listed across {len(WANT)} functions')
    return 0


if __name__ == '__main__':
    sys.exit(main())
