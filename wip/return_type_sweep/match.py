"""Pair each target function to its draft function using the SAME content-based
greedy algorithm as wip/wm_units/verify_anon.py (the tool this whole project
already trusts for anonymous-symbol WM units), then report the INSTRUCTION
COUNT DELTA for every pair.

Why not harness.extract() by name: almost every function in these WM-family
targets is an anonymous `fn_2_XXXXXX` placeholder (dtk could not resolve a
symbol), and a placeholder's "name" bakes in its address, which is different
in a standalone single-file compile than in the linked REL. A name-based
diff therefore matches almost nothing here, as a first pass confirmed (1638 of
1667 functions read as "unauthored" purely from the naming artefact, not from
missing work). verify_anon.py's own content-based pairing is the tool actually
used throughout this project's HANDOFF for exactly this family, so this reuses
it rather than inventing a fourth diffing method.

Writes only into wip/return_type_sweep/.
"""
import os
import re
import sys
import glob
import json

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'wip', 'wm_units'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import verify_anon as VA  # noqa: E402
import harness as H  # noqa: E402

WM_UNITS = os.path.join(ROOT, 'wip', 'wm_units')
BUILD = os.path.join(ROOT, 'wip', 'return_type_sweep', 'build')

UNITS = [
    'agent_river', 'agent_koopajr', 'agent_kinopio', 'agent_castle_bg',
    'agent_castle', 'agent_koopa_castle', 'agent_floor_jr_a', 'agent_water_move',
    'agent_antlion_mng', 'agent_hanachan', 'agent_antlion', 'agent_anchor',
    'agent_board', 'agent_course', 'agent_ghost', 'agent_gun_battery',
    'agent_item', 'agent_killer', 'agent_killerbullet', 'agent_kinoballoon',
    'agent_kinoko_base', 'agent_kinoko_red', 'agent_kinoko_star', 'agent_manta',
    'agent_nice_coin', 'agent_note', 'agent_sandpillar', 'agent_sinkship',
    'agent_smallcloud', 'agent_start', 'agent_dance_pakkun',
]

EXCLUDE_PREFIXES = ('probe', 'v', 'layout_check', 'off_probe', 'draw_probe')

CASTLE_TARGET_OBJS = [
    'bin/dtkspl/d_basesNP/obj/auto_00_0015ECC0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_15FAE0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_0015FBB4_text.o',
]


def find_target_files(unit_dir):
    out = []
    for p in glob.glob(os.path.join(unit_dir, '*.txt')):
        base = os.path.basename(p)
        if base == 'draft.txt':
            continue
        low = base.lower()
        if any(low.startswith(pfx) for pfx in EXCLUDE_PREFIXES):
            continue
        if 'compiled' in low or 'dokan_test' in low:
            continue
        out.append(p)
    return sorted(out)


def is_text_disasm(path):
    try:
        with open(path, encoding='utf-8', errors='replace') as fh:
            for i, line in enumerate(fh):
                if '.fn ' in line:
                    return True
                if i > 200:
                    break
    except OSError:
        return False
    return False


def match_unit(target_funcs, draft_funcs):
    """target_funcs: [(addr, name, ins)] sorted by addr, pad_/gap_ excluded.
    draft_funcs: [(name, ins)] in emission (= source definition) order.

    Returns a list of dicts, one per target function, same algorithm as
    verify_anon.main(): exact/tail-blr matches consume the earliest ascending
    unused draft index; everything else gets the nearest-by-content-diff
    unused draft candidate, reported but NOT consumed (informational only,
    exactly as verify_anon does it -- consuming it would let one draft function
    silently claim multiple targets).
    """
    used = set()
    last = -1
    out = []
    for addr, name, ins in target_funcs:
        want = VA.norm(ins)
        candidates = [i for i, (dname, dins) in enumerate(draft_funcs)
                      if i not in used and VA.eq_mod_tail_blr(want, VA.norm(dins))]
        ascending = [i for i in candidates if i > last]
        hit = ascending[0] if ascending else (candidates[0] if candidates else None)
        if hit is not None:
            last = hit
            used.add(hit)
            out.append({
                'addr': addr, 'name': name, 'target_len': len(ins),
                'match': True, 'draft_name': draft_funcs[hit][0],
                'draft_len': len(draft_funcs[hit][1]), 'delta': 0,
                'diffcount': 0,
            })
            continue
        best_i, best_score = None, None
        for i, (dname, dins) in enumerate(draft_funcs):
            if i in used:
                continue
            a, b = VA.norm(dins), want
            n = sum(1 for j in range(max(len(a), len(b)))
                    if (a[j] if j < len(a) else None) != (b[j] if j < len(b) else None))
            if best_score is None or n < best_score:
                best_score, best_i = n, i
        if best_i is not None:
            dlen = len(draft_funcs[best_i][1])
            out.append({
                'addr': addr, 'name': name, 'target_len': len(ins),
                'match': False, 'draft_name': draft_funcs[best_i][0],
                'draft_len': dlen, 'delta': dlen - len(ins),
                'diffcount': best_score,
            })
        else:
            out.append({
                'addr': addr, 'name': name, 'target_len': len(ins),
                'match': False, 'draft_name': None, 'draft_len': None,
                'delta': None, 'diffcount': None,
            })
    return out


def main():
    report = {}
    for unit in UNITS:
        unit_dir = os.path.join(WM_UNITS, unit)
        my_dir = os.path.join(BUILD, unit)
        draft_txt = os.path.join(my_dir, 'draft.txt')
        if not os.path.exists(draft_txt):
            report[unit] = {'error': 'no compiled draft.txt (see sweep.py output)'}
            continue

        target_paths = [p for p in find_target_files(unit_dir) if is_text_disasm(p)]
        if unit == 'agent_castle':
            for objp in CASTLE_TARGET_OBJS:
                outp = os.path.join(my_dir, os.path.basename(objp) + '.txt')
                if os.path.exists(outp):
                    target_paths.append(outp)

        if not target_paths:
            report[unit] = {'error': 'no target text files found', 'checked_dir': unit_dir}
            continue

        target_funcs = []
        seen_addr = set()
        for tp in target_paths:
            for addr, name, ins in VA.functions(tp, with_addr=True):
                if addr in seen_addr:
                    continue
                seen_addr.add(addr)
                target_funcs.append((addr, name, ins))
        target_funcs.sort(key=lambda t: t[0])

        draft_funcs = VA.functions(draft_txt)

        pairs = match_unit(target_funcs, draft_funcs)
        report[unit] = {
            'target_files': [os.path.basename(p) for p in target_paths],
            'n_target': len(target_funcs),
            'n_draft': len(draft_funcs),
            'pairs': pairs,
        }
        n_match = sum(1 for p in pairs if p['match'])
        print('%-22s target=%-4d draft=%-4d exact/tailblr=%-4d' % (
            unit, len(target_funcs), len(draft_funcs), n_match))

    out_json = os.path.join(ROOT, 'wip', 'return_type_sweep', 'match_raw.json')
    with open(out_json, 'w', encoding='utf-8') as fh:
        json.dump(report, fh, indent=1)
    print('\nWrote', out_json)


if __name__ == '__main__':
    main()
