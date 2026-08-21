"""Diagnostic: full unmatched list (no top-25 cut), plus per-function diff detail.
Lives in wip/gapA/b1_sweep/ only; imports tally.py's helpers without modifying it.
Usage: python full_todo.py <draft.cpp> <shadow_include_dir> [name_substr]
"""
import os, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'tools', 'auto_decomp'))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'line_mng_shared'))
import harness
import tally as T

def main():
    src = os.path.abspath(sys.argv[1])
    inc = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else os.path.join(T.HERE, 'shadow_include')
    filt = sys.argv[3] if len(sys.argv) > 3 else None
    work = os.path.join(os.path.dirname(src), '_tally_full')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)
    d, t = T.parse(txt), T.parse(T.TARGET)
    hit = [k for k in t if k in d and T.matched(d[k], t[k])]
    used = set(hit)
    spare = {k: v for k, v in d.items() if k not in t}
    spare_by_bytes = {}
    for k, v in spare.items():
        spare_by_bytes.setdefault(tuple(b for b, _ in v), []).append(k)
    for k in t:
        if k in used:
            continue
        cand = spare_by_bytes.get(tuple(b for b, _ in t[k]))
        if cand:
            hit.append(k); used.add(k)
    todo = sorted(((len(v), k, len(d[k]) if k in d else None)
                   for k, v in t.items() if k not in hit), reverse=True)
    print(f'matched {len(hit)}/{len(t)} functions')
    for n, k, dn in todo:
        if filt and filt not in k:
            continue
        state = 'MISSING' if dn is None else ('LEN OK' if dn == n else f'{dn}w vs {n}w  STRUCTURAL')
        print(f'{n:5d}w  {state:>18}  {k}')
    if filt:
        # show detailed diff for matches of filt
        for k in t:
            if filt in k and k not in hit and k in d:
                print(f'\n--- DIFF for {k} ---')
                dv, tv = d[k], t[k]
                for i in range(max(len(dv), len(tv))):
                    db = dv[i][0] if i < len(dv) else '(missing)'
                    dt = dv[i][1] if i < len(dv) else ''
                    tb = tv[i][0] if i < len(tv) else '(missing)'
                    tt = tv[i][1] if i < len(tv) else ''
                    mark = '   ' if db == tb else '!! '
                    print(f'{mark}{i:3d}  D: {db:30s} {dt:40s}  T: {tb:30s} {tt}')
    return 0

if __name__ == '__main__':
    sys.exit(main())
