"""Return-type sweep: for each parked/in-progress WM unit, compile the existing
draft standalone and compare per-function INSTRUCTION COUNTS against every
target_*.txt in that unit's directory.

Writes nothing into any agent_* directory. Draft objects/disassembly land in
wip/return_type_sweep/build/<unit>/.

Usage: python wip/return_type_sweep/sweep.py
"""
import os
import sys
import json
import glob

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness as H

WM_UNITS = os.path.join(ROOT, 'wip', 'wm_units')
OUT_BUILD = os.path.join(ROOT, 'wip', 'return_type_sweep', 'build')

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

# agent_castle keeps no saved target_*.txt at all -- its own iterate.py
# disassembles these three original split objects fresh every round instead.
# Same recipe here, output written into OUR OWN build dir, never into
# agent_castle/.
CASTLE_TARGET_OBJS = [
    'bin/dtkspl/d_basesNP/obj/auto_00_0015ECC0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_fn_2_15FAE0_text.o',
    'bin/dtkspl/d_basesNP/obj/auto_00_0015FBB4_text.o',
]


def find_draft_cpp(unit_dir, unit_name):
    """The unit draft is named after the unit, not a probe/experiment file."""
    cands = glob.glob(os.path.join(unit_dir, '*.cpp'))
    good = []
    for c in cands:
        base = os.path.basename(c)
        low = base.lower()
        if any(low.startswith(p) for p in EXCLUDE_PREFIXES):
            continue
        good.append(c)
    # Prefer a name containing d_a_ (the real actor source file convention)
    da = [c for c in good if 'd_a_' in os.path.basename(c).lower()]
    if da:
        # if several, prefer the one NOT ending in a baseline/backup suffix
        da = [c for c in da if os.path.basename(c).count('.cpp') == 1
              and os.path.basename(c).endswith('.cpp')]
        if da:
            return da[0]
    return good[0] if good else None


def find_include_dir(unit_dir):
    for name in ('shadow_include', 'include'):
        p = os.path.join(unit_dir, name)
        if os.path.isdir(p):
            return p
    return None


def find_target_files(unit_dir):
    """Every *.txt in the unit dir that could be a target disassembly, minus
    draft.txt and known non-target dumps (probes, compiled-draft echoes). The
    actual filter for "is this really a dtk .text disassembly" is
    is_text_disasm(), applied by the caller -- this just avoids opening files
    we already know are not candidates."""
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
                if '.fn ' in line or line.strip().startswith('.fn'):
                    return True
                if i > 200:
                    break
    except OSError:
        return False
    return False


def main():
    os.makedirs(OUT_BUILD, exist_ok=True)
    report = {}

    for unit in UNITS:
        unit_dir = os.path.join(WM_UNITS, unit)
        if not os.path.isdir(unit_dir):
            report[unit] = {'error': 'directory not found'}
            continue

        src = find_draft_cpp(unit_dir, unit)
        if not src:
            report[unit] = {'error': 'no draft .cpp found'}
            continue

        inc = find_include_dir(unit_dir)
        extra_inc = [inc] if inc else []

        my_dir = os.path.join(OUT_BUILD, unit)
        os.makedirs(my_dir, exist_ok=True)
        obj = os.path.join(my_dir, 'draft.o')
        txt = os.path.join(my_dir, 'draft.txt')

        ok, log = H.compile_draft(src, obj, extra_inc=extra_inc, module='d_basesNP')
        if not ok:
            report[unit] = {'error': 'COMPILE FAILED', 'src': src, 'log': log[-3000:]}
            continue

        ok2, log2 = H.disasm(obj, txt)
        if not ok2:
            report[unit] = {'error': 'DISASM FAILED', 'src': src, 'log': log2[-3000:]}
            continue

        target_files = find_target_files(unit_dir)
        text_targets = [t for t in target_files if is_text_disasm(t)]

        # agent_castle keeps no saved target text at all; disassemble the
        # three original split objects its own iterate.py uses, into OUR
        # build dir (never into agent_castle/).
        if unit == 'agent_castle':
            for objp in CASTLE_TARGET_OBJS:
                obj_abs = os.path.join(ROOT, objp)
                outp = os.path.join(my_dir, os.path.basename(objp) + '.txt')
                okc, logc = H.disasm(obj_abs, outp)
                if okc:
                    text_targets.append(outp)
                else:
                    print('  castle target disasm FAILED for', objp, logc[-500:])

        funcs = {}  # name -> (target_file, target_len, draft_len or None)
        for tgt in text_targets:
            names = H.list_functions(tgt)
            for name in names:
                if name in funcs:
                    continue  # already found in an earlier (or same) target file
                want = H.extract(tgt, name)
                if want is None:
                    continue
                got = H.extract(txt, name)
                funcs[name] = {
                    'target_file': os.path.basename(tgt),
                    'target_len': len(want),
                    'draft_len': (len(got) if got is not None else None),
                }

        report[unit] = {
            'src': src,
            'include': inc,
            'target_files': [os.path.basename(t) for t in text_targets],
            'functions': funcs,
        }
        print('%-22s src=%-40s targets=%d funcs=%d' % (
            unit, os.path.basename(src), len(text_targets), len(funcs)))

    out_json = os.path.join(ROOT, 'wip', 'return_type_sweep', 'sweep_raw.json')
    with open(out_json, 'w', encoding='utf-8') as fh:
        json.dump(report, fh, indent=1)
    print('\nWrote', out_json)


if __name__ == '__main__':
    main()
