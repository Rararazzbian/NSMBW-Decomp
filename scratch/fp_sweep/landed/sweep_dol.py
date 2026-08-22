"""Full poolcheck+blcheck sweep of every LANDED wiimj2d.dol unit in
slices/wiimj2d.json, using retail split objects from bin/dtkspl/obj and the
already-compiled objects in bin/compiled/wiimj2d/.

Read-only on source/, bin/compiled, bin/dtkspl. Writes only into
scratch/fp_sweep/landed/_dis (retail disasm cache) and prints a report,
also saved to scratch/fp_sweep/landed/dol_report.txt.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
TOOLS = os.path.join(ROOT, 'tools', 'auto_decomp')
sys.path.insert(0, TOOLS)
sys.path.insert(0, os.path.dirname(HERE))
sys.path.insert(0, HERE)
import harness
import poolcheck
import pool as poolmod
import blcheck
from index_dtkspl import build_index, overlapping

DISCACHE = os.path.join(HERE, '_dis')
os.makedirs(DISCACHE, exist_ok=True)

# Extend poolcheck's POOL_REF to ALSO catch the two-instruction @ha/@l form,
# used for larger .rodata objects. poolcheck.py's own regex only matches the
# single-instruction @sda21/@sda2 form -- confirmed blind to @ha/@l this
# session, and confirmed to hide at least one real defect (d_a_wm_ghost.cpp,
# an unlanded draft). This checks whether that pattern recurs in LANDED code.
HA_REF = re.compile(r'^\s*lis\s+r(\d+),\s*"?(@[\w]+)"?@ha\s*$')
L_REF = re.compile(r'^\s*(lfs|lfd)\s+f\d+,\s*"?(@[\w]+)"?@l\(r(\d+)\)')


def find_ha_l_pairs(fn):
    """[(index_of_l, width, symbol)] for lis @ha ... lfs/lfd @l(rN) pairs
    where rN was set by a preceding lis to the SAME symbol (a few
    instructions earlier -- the ha and l need not be adjacent)."""
    pending = {}  # reg -> symbol
    out = []
    for i, (_, text) in enumerate(fn):
        mh = HA_REF.match(text)
        if mh:
            pending[mh.group(1)] = mh.group(2)
            continue
        ml = L_REF.match(text)
        if ml and pending.get(ml.group(3)) == ml.group(2):
            width = 4 if ml.group(1) == 'lfs' else 8
            out.append((i, width, ml.group(2)))
    return out


def load_units():
    units = []
    for modname, jsonfile in [('wiimj2d', 'wiimj2d.json')]:
        d = json.load(open(os.path.join(ROOT, 'slices', jsonfile), encoding='utf-8'))
        text_base = int(d['meta']['sections']['.text']['addr'], 16)
        for u in d['slices']:
            tr = u['memoryRanges'].get('.text')
            if not tr:
                continue
            lo_s, hi_s = tr.split('-')
            lo, hi = text_base + int(lo_s, 16), text_base + int(hi_s, 16)
            units.append((modname, u['source'], lo, hi))
    return units


def disasm_cached(obj_path):
    base = os.path.splitext(os.path.basename(obj_path))[0]
    out = os.path.join(DISCACHE, base + '.txt')
    if not os.path.exists(out) or os.path.getsize(out) == 0:
        ok, log = harness.disasm(obj_path, out)
        if not ok:
            return None
    return out


def main():
    units = load_units()
    idx, unnamed_sinit = build_index()
    print(f'{len(units)} landed wiimj2d.dol units in slices/wiimj2d.json')
    print(f'{len(idx)} addressed retail split objects indexed, '
          f'{len(unnamed_sinit)} name-only (__sinit) objects not indexed by address\n')

    total_pool_checked = total_pool_mismatch = 0
    total_bl_checked = total_bl_mismatch = 0
    total_hal_found = 0
    n_ok = n_no_retail_obj = n_no_compiled_obj = n_compile_or_disasm_fail = 0
    findings = []
    hal_report = []
    dol = poolmod.load()

    for i, (modname, source, lo, hi) in enumerate(units, 1):
        draft_o = os.path.join(ROOT, 'bin', 'compiled', 'wiimj2d',
                                os.path.splitext(source)[0] + '.o')
        if not os.path.exists(draft_o):
            n_no_compiled_obj += 1
            continue
        retail_objs = overlapping(idx, lo, hi)
        if not retail_objs:
            n_no_retail_obj += 1
            continue
        retail_txts = []
        ok_all = True
        for ro in retail_objs:
            t = disasm_cached(ro)
            if t is None:
                ok_all = False
                break
            retail_txts.append(t)
        if not ok_all:
            n_compile_or_disasm_fail += 1
            continue
        draft_txt = disasm_cached(draft_o)
        if draft_txt is None:
            n_compile_or_disasm_fail += 1
            continue

        draft = poolcheck.parse_fns(draft_txt)
        target = {}
        for t in retail_txts:
            target.update(poolcheck.parse_fns(t))
        dpool = poolcheck.object_pool(draft_o)

        pairs = []
        for tname in target:
            if tname in draft:
                pairs.append((tname, tname))
                continue
            cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
            if cand:
                pairs.append((tname, cand))

        n_ok += 1
        for tname, dname in pairs:
            t, d = target[tname], draft[dname]
            if len(t) != len(d):
                continue
            raw_eq = [b for b, _ in t] == [b for b, _ in d]
            canon_eq = (harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
            gate_matched = raw_eq or canon_eq
            if not gate_matched:
                continue

            bad_pool = poolcheck.compare_pools(t, d, dpool, dol)
            total_pool_checked += sum(1 for _, tx in t if poolcheck.POOL_REF.match(tx))
            for idxp, va, tv, dv in bad_pool:
                total_pool_mismatch += 1
                findings.append(('POOL', source, tname, idxp, f'0x{va:08X}', tv, dv,
                                  raw_eq, canon_eq))

            bad_bl = blcheck.compare_bl(t, d)
            total_bl_checked += sum(1 for _, tx in t if blcheck.BL_REF.match(tx))
            for idxb, tt, dt in bad_bl:
                total_bl_mismatch += 1
                findings.append(('BL', source, tname, idxb, '-', tt, dt, raw_eq, canon_eq))

            hal = find_ha_l_pairs(t)
            if hal:
                total_hal_found += len(hal)
                hal_report.append((source, tname, len(hal)))

        if i % 20 == 0:
            print(f'  ...{i}/{len(units)} units processed')

    print(f'\n=== DOL SUMMARY ===')
    print(f'units with both a compiled object and a retail dump, actually checked: {n_ok}')
    print(f'skipped, no compiled object in bin/compiled/wiimj2d: {n_no_compiled_obj}')
    print(f'skipped, no overlapping retail split object in bin/dtkspl/obj: {n_no_retail_obj}')
    print(f'skipped, disasm failure: {n_compile_or_disasm_fail}')
    print(f'\nPOOL: {total_pool_checked} lfs/lfd @sda21 refs compared by value, '
          f'{total_pool_mismatch} mismatched')
    print(f'BL  : {total_bl_checked} bl/bla refs compared by target name, '
          f'{total_bl_mismatch} mismatched')
    print(f'\n@ha/@l pool-form refs found (NOT checked by poolcheck.py, blind spot): '
          f'{total_hal_found} across {len(hal_report)} functions')
    if hal_report:
        print('Sample (up to 15):')
        for src, fn, n in hal_report[:15]:
            print(f'  {n:3d}  {src}  {fn}')

    print()
    for kind, source, fn, idxp, addr, want, got, raw_eq, canon_eq in findings:
        gate = 'RAW-BYTES-ONLY (dangerous)' if raw_eq and not canon_eq else 'raw+canon both'
        print(f'{kind} MISMATCH: {source}  {fn}  instr {idxp}  retail={want!r} draft={got!r}  '
              f'addr={addr}  gate={gate}')

    return {
        'n_units': len(units), 'n_ok': n_ok, 'n_no_compiled': n_no_compiled_obj,
        'n_no_retail': n_no_retail_obj, 'n_fail': n_compile_or_disasm_fail,
        'pool_checked': total_pool_checked, 'pool_mismatch': total_pool_mismatch,
        'bl_checked': total_bl_checked, 'bl_mismatch': total_bl_mismatch,
        'hal_count': total_hal_found, 'findings': findings,
    }


if __name__ == '__main__':
    r = main()
    out = os.path.join(HERE, 'dol_report.json')
    with open(out, 'w', encoding='utf-8') as f:
        json.dump(r, f, indent=2)
    print(f'\nWritten: {out}')
