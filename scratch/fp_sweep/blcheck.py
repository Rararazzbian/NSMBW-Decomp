"""Compare a draft's `bl` CALL TARGETS against retail's, by resolved symbol name.

Why this exists
----------------
poolcheck.py closed the wrong-constant hole for `lfs`/`lfd`, but the exact same
zeroed-relocation defect applies to `bl`. A `bl` to ANY target assembles to the
SAME raw bytes on both sides -- `48 00 00 01` (opcode + link bit, displacement
ZEROED) -- confirmed by inspecting real disassembly output, e.g.:

    /* 00000010 00000050  48 00 00 01 */  bl __nw__7fBase_cFUl
    /* 0000001C 0000005C  48 00 00 01 */  bl __ct__17daWmKoopaCastle_cFv

Both lines have byte pattern `48 00 00 01`. So a function that is byte-identical
in every other instruction, but calls the WRONG function, is RAW-BYTE-IDENTICAL
to the correct one.

This matters most for any gate that accepts a "raw bytes equal" verdict WITHOUT
also requiring canonicalised-text equality -- `wip/line_mng_shared/tally.py`'s
`matched()` and `poolcheck.py`'s own `gate_matched` line both do exactly that
(`bytes_equal OR canon_text_equal`, with the OR short-circuiting on the bytes
check). Raw-byte equality is blind to a wrong callee for the same reason it is
blind to a wrong float; canonicalised TEXT is not blind here (it keeps the
resolved symbol name), so a function accepted ONLY via the raw-byte half of
that union is specifically at risk.

    python blcheck.py <draft.cpp> <shadow_include> <target.txt>
    python blcheck.py ... --all       also check functions that already differ

By default only functions the union gate calls MATCHED are checked (same
default as poolcheck.py), because those are the dangerous ones.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.join(os.path.dirname(os.path.dirname(HERE)), 'tools', 'auto_decomp')
sys.path.insert(0, TOOLS)
import harness
import poolcheck  # reuse parse_fns

BL_REF = re.compile(r'^\s*(bl|bla)\s+"?([^\s,"]+)"?\s*$')
ADDR_SUFFIX = re.compile(r'_[0-9A-Fa-f]{8}$')
PLACEHOLDER = re.compile(r'^(?:fn|func|lbl)_[0-9A-Fa-f]{8}$')
# Same shape as harness.PLACEHOLDER_CALLEE: a target-side unnamed placeholder
# (fn_800C1EE0) and a draft-side STATIC function in the SAME translation unit
# both resolve to the SAME address, but dtk's draft-side disassembly appends
# the CFront mangling (fn_800C1EE0__FP10dLineMng_cff...) because the draft
# compiles it as a real (if file-local) symbol. That is not a different
# callee -- it is the identical function, spelled two ways depending on which
# object it was disassembled from. Strip the suffix from a placeholder name
# before comparing, exactly as harness.canonicalise does for the main gate.
# Getting this wrong self-reported 29 phantom "wrong callee" hits on
# functions that are provably correct (first cut of this tool, corrected
# after cross-checking against harness.canonicalise on the same lines).
PLACEHOLDER_MANGLED = re.compile(r'^((?:fn|func|lbl)_[0-9A-Fa-f]{8})__\w+$')


def norm_target(name):
    """Normalise a bl operand for comparison across target/draft disassembly.

    Strips dtk's duplicate-disambiguating _ADDR suffix from an ordinary named
    symbol, and strips a CFront mangling suffix from an unnamed fn_/func_/lbl_
    placeholder (the address is that placeholder's identity, not its name)."""
    name = name.strip().strip('"')
    m = PLACEHOLDER_MANGLED.match(name)
    if m:
        return m.group(1)
    if PLACEHOLDER.match(name):
        return name
    return ADDR_SUFFIX.sub('', name)


def bl_targets(fn):
    """[(index, normalised_target, raw_operand)] for every bl/bla in a
    [(bytes, text), ...] function body."""
    out = []
    for i, (_, text) in enumerate(fn):
        m = BL_REF.match(text)
        if m:
            out.append((i, norm_target(m.group(2)), m.group(2)))
    return out


def compare_bl(target_fn, draft_fn):
    """[(index, retail_target, draft_target)] for every position where BOTH
    sides have a bl/bla and the normalised targets disagree. Position-aligned,
    exactly like poolcheck.compare_pools -- valid because the caller has
    already established the two function bodies are the same length under the
    gate (raw bytes equal or canonicalised text equal)."""
    out = []
    n = min(len(target_fn), len(draft_fn))
    for i in range(n):
        tm = BL_REF.match(target_fn[i][1])
        dm = BL_REF.match(draft_fn[i][1])
        if not tm or not dm:
            continue
        tt, dt = norm_target(tm.group(2)), norm_target(dm.group(2))
        if tt != dt:
            out.append((i, tt, dt))
    return out


def main():
    args = [a for a in sys.argv[1:] if a != '--all']
    only_matched = len(args) == len(sys.argv[1:])
    if len(args) < 3:
        print(__doc__)
        return 2
    src, inc, target_txt = (os.path.abspath(a) for a in args[:3])

    work = os.path.join(os.path.dirname(src), '_blcheck')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)

    draft, target = poolcheck.parse_fns(txt), poolcheck.parse_fns(target_txt)

    pairs = []
    for tname in target:
        if tname in draft:
            pairs.append((tname, tname))
            continue
        cand = next((d for d in draft if '__' in d and d.split('__')[0] == tname), None)
        if cand:
            pairs.append((tname, cand))

    checked = mismatched = 0
    findings = []
    for tname, dname in pairs:
        t, d = target[tname], draft[dname]
        if len(t) != len(d):
            continue
        gate_matched = ([b for b, _ in t] == [b for b, _ in d]
                        or harness.canonicalise([x for _, x in t])
                        == harness.canonicalise([x for _, x in d]))
        if only_matched and not gate_matched:
            continue
        checked += len(bl_targets(t))
        bad = compare_bl(t, d)
        for i, tt, dt in bad:
            mismatched += 1
            findings.append((tname, i, tt, dt, gate_matched))

    for name, i, tt, dt, gm in findings:
        flag = 'FALSE POSITIVE (wrong callee)' if gm else 'differing fn'
        print(f'{flag}: {name}')
        print(f'    instruction {i}: retail calls {tt!r}   draft calls {dt!r}')
    print(f'\n{checked} bl/bla call targets compared by NAME across '
          f'{len(pairs)} paired functions')
    print(f'{mismatched} mismatched')
    return 1 if mismatched else 0


if __name__ == '__main__':
    sys.exit(main())
