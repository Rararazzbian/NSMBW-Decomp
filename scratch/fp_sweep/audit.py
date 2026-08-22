"""Generic false-positive auditor: pooled-constant VALUE check (poolcheck) +
bl CALL-TARGET name check (blcheck), driven off whatever artifacts already
exist for a unit, without needing a single merged target.txt.

Usage:
    python audit.py --draft-o <draft.o>            [--draft-cpp <src> --inc <dir>]
                     --target <t1.txt> [<t2.txt> ...]
                     [--all]

Exactly one of --draft-o (an ALREADY-COMPILED object, read-only) or
--draft-cpp (compiled fresh into scratch/fp_sweep/_work/<label>/) must be given.
Multiple --target files (e.g. per-split-object retail dumps) are merged.

Only functions the union gate (raw bytes OR canonicalised text) calls MATCHED
are checked by default -- pass --all to also see already-differing functions.

This never writes anywhere except scratch/fp_sweep/_work/ and never touches a
draft source file.
"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.join(os.path.dirname(os.path.dirname(HERE)), 'tools', 'auto_decomp')
sys.path.insert(0, TOOLS)
import harness
import poolcheck
import pool as poolmod
import blcheck


def merged_parse(paths):
    """{name: [(bytes, text), ...]} across several target dump files.

    A later file's function silently overrides an earlier one only if the
    name repeats; report duplicates so a caller can decide if that is
    expected (adjacent split objects sometimes both carry a stub / gap_ name).
    """
    out = {}
    dupes = []
    for p in paths:
        fns = poolcheck.parse_fns(p)
        for k, v in fns.items():
            if k in out and out[k] != v:
                dupes.append(k)
            out[k] = v
    return out, dupes


def object_pool_ro(obj_path):
    return poolcheck.object_pool(obj_path)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--draft-o')
    ap.add_argument('--draft-cpp')
    ap.add_argument('--inc', action='append', default=[])
    ap.add_argument('--module', default='wiimj2d')
    ap.add_argument('--target', nargs='+', required=True)
    ap.add_argument('--all', action='store_true')
    ap.add_argument('--label', default='unit')
    args = ap.parse_args()

    if not args.draft_o and not args.draft_cpp:
        sys.exit('need --draft-o or --draft-cpp')

    if args.draft_cpp:
        work = os.path.join(HERE, '_work', args.label)
        os.makedirs(work, exist_ok=True)
        obj = os.path.join(work, 'd.o')
        ok, err = harness.compile_draft(args.draft_cpp, obj, extra_inc=args.inc,
                                         module=args.module)
        if not ok:
            print('COMPILE FAILED\n' + err)
            return 1
    else:
        obj = args.draft_o
        work = os.path.join(HERE, '_work', args.label)
        os.makedirs(work, exist_ok=True)

    txt = os.path.join(work, 'd.txt')
    dok, dlog = harness.disasm(obj, txt)
    if not dok:
        print('DISASM FAILED\n' + dlog)
        return 1

    draft = poolcheck.parse_fns(txt)
    target, dupes = merged_parse(args.target)
    if dupes:
        print(f'NOTE: {len(dupes)} function name(s) appear in more than one '
              f'--target file with DIFFERENT bodies (kept the last): '
              + ', '.join(dupes[:10]))

    dpool = object_pool_ro(obj)
    dol = poolmod.load()

    pairs = []
    for tname in target:
        if tname in draft:
            pairs.append((tname, tname))
            continue
        cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
        if cand:
            pairs.append((tname, cand))

    pairing_mode = 'name'
    # Retail's DOL/REL is a linked, stripped binary: most non-exported
    # functions have NO symbol name, so dtk invents fn_2_<ADDR>/lbl_<ADDR>
    # placeholders for them. When that is most of the target (typical for a
    # whole actor class with no external callers), name-pairing finds almost
    # nothing and silently checks nothing. Fall back to PLACEMENT ORDER --
    # the same technique wip/wm_units/check_vtable.py uses -- pairing the
    # target's real (non pad_/gap_) functions in file/address order against
    # the draft's functions in definition order. This is only valid because
    # this project enforces "function DEFINITION ORDER is part of the
    # object" (AGENT_CONTEXT.md) -- draft order should equal target order
    # for any unit actually claiming to be near-complete.
    real_target_order = [n for n in target
                          if not n.startswith('pad_') and not n.startswith('gap_')]
    if len(pairs) < 0.2 * max(len(real_target_order), 1):
        pairing_mode = 'positional (retail names are stripped placeholders)'
        draft_order = list(draft.keys())
        pairs = list(zip(real_target_order, draft_order))
        if len(real_target_order) != len(draft_order):
            print(f'NOTE: positional pairing with UNEQUAL counts -- '
                  f'{len(real_target_order)} target vs {len(draft_order)} draft '
                  f'functions. Only the first {len(pairs)} were paired; this '
                  f'unit is not actually function-count-complete, or contains '
                  f'functions from a neighbouring TU (see check_fn_order.py).')

    checked_pool = mismatched_pool = 0
    checked_bl = mismatched_bl = 0
    pool_findings = []
    bl_findings = []
    n_paired = n_len_ok = n_gate_matched = 0

    for tname, dname in pairs:
        t, d = target[tname], draft[dname]
        n_paired += 1
        if len(t) != len(d):
            continue
        n_len_ok += 1
        gate_matched = ([b for b, _ in t] == [b for b, _ in d]
                        or harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
        if gate_matched:
            n_gate_matched += 1
        if not args.all and not gate_matched:
            continue

        for i, va, tv, dv in poolcheck.compare_pools(t, d, dpool, dol):
            mismatched_pool += 1
            pool_findings.append((tname, i, va, tv, dv, gate_matched))
        checked_pool += sum(1 for _, tx in t if poolcheck.POOL_REF.match(tx))

        bad_bl = blcheck.compare_bl(t, d)
        for i, tt, dtv in bad_bl:
            mismatched_bl += 1
            bl_findings.append((tname, i, tt, dtv, gate_matched))
        checked_bl += sum(1 for _, tx in t if blcheck.BL_REF.match(tx))

    print(f'=== {args.label} ===')
    print(f'pairing mode: {pairing_mode}')
    print(f'{len(target)} target functions, {n_paired} paired, '
          f'{n_len_ok} same length, {n_gate_matched} gate-MATCHED')
    print()
    for name, i, va, tv, dv, gm in pool_findings:
        flag = 'FALSE POSITIVE (wrong constant)' if gm else 'differing fn, wrong constant'
        print(f'{flag}: {name}')
        print(f'    instr {i}: retail 0x{va:08X} = {tv!r}   draft = {dv!r}')
    for name, i, tt, dtv, gm in bl_findings:
        flag = 'FALSE POSITIVE (wrong callee)' if gm else 'differing fn, wrong callee'
        print(f'{flag}: {name}')
        print(f'    instr {i}: retail calls {tt!r}   draft calls {dtv!r}')
    print()
    print(f'POOL: {checked_pool} lfs/lfd compared by value, {mismatched_pool} mismatched')
    print(f'BL  : {checked_bl} bl/bla compared by target name, {mismatched_bl} mismatched')
    return 1 if (pool_findings or bl_findings) else 0


if __name__ == '__main__':
    sys.exit(main())
