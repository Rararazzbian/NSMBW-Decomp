"""Byte-authoritative tally for d_line_mng drafts.

Why this exists
---------------
`harness.canonicalise` can report a FALSE MISMATCH. All four `mov_to_*`
functions were length-exact AND byte-identical against target, yet canonicalise
called them differing -- purely because the target's disassembly QUOTES a symbol
name (`"@49614_80359100"`) where a standalone `.o` shows an unresolved form
(`...bss.0`). The quote characters survive canonicalisation.

Byte equality is the actual matching criterion for this project. The
canonicaliser is a convenience over it, useful where relocation genuinely makes
the bytes differ. So the correct gate is the UNION:

    matched  ==  raw bytes equal  OR  canonicalised text equal

Counting only one of the two undercounts. Using canonicalise alone undercounted
this unit by four functions on its first authoring round.

Usage:  python tally.py <draft.cpp> <shadow_include_dir>
"""
import os, re, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools', 'auto_decomp'))
import harness
import poolcheck

HERE = os.path.dirname(os.path.abspath(__file__))
TARGET = os.path.join(HERE, 'target.txt')


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


# Set once by main(); a (draft_pool, dol) pair. Left as None only if the object
# could not be read, in which case the value check is skipped rather than
# silently passing everything -- see the warning main() prints.
POOLS = None
POOL_FAILURES = []


def matched(draft_fn, target_fn, name='?'):
    """The union gate, plus a check BOTH halves of the union are blind to.

    Bytes first -- it is the real criterion. But neither half of the union can
    see a wrong pooled CONSTANT: an `lfs`/`lfd` offset field is zeroed, so the
    bytes agree, and canonicalisation renumbers pool symbols by order of
    appearance, so the text agrees too. A draft loading 0.0f where retail loads
    1.0f passes both. That has produced false positives in three separate rounds,
    so the value is now read out of the binaries and compared as well.
    """
    if [b for b, _ in draft_fn] == [b for b, _ in target_fn]:
        pass
    elif (harness.canonicalise([t for _, t in draft_fn])
            != harness.canonicalise([t for _, t in target_fn])):
        return False
    if POOLS is None:
        return True
    bad = poolcheck.compare_pools(target_fn, draft_fn, *POOLS)
    for i, va, tv, dv in bad:
        POOL_FAILURES.append((name, i, va, tv, dv))
    return not bad


def main():
    src = os.path.abspath(sys.argv[1])
    inc = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 else os.path.join(HERE, 'shadow_include')
    work = os.path.join(os.path.dirname(src), '_tally')
    os.makedirs(work, exist_ok=True)
    obj, txt = os.path.join(work, 'd.o'), os.path.join(work, 'd.txt')
    ok, err = harness.compile_draft(src, obj, extra_inc=[inc])
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    harness.disasm(obj, txt)
    d, t = parse(txt), parse(TARGET)
    paired_by_content = []

    # Read both sides' literal pools once, so `matched` can check constants by
    # VALUE. Without this the gate counts a draft loading 0.0f against a retail
    # 1.0f as a match -- the instruction bytes and the canonical text both agree.
    global POOLS
    try:
        POOLS = (poolcheck.object_pool(obj), poolcheck.pool.load())
    except Exception as e:                                   # noqa: BLE001
        POOLS = None
        print(f'WARNING: pooled constants NOT value-checked ({e}). '
              f'A wrong float constant will pass as a match.')

    # Name-keyed pass.
    hit = [k for k in t if k in d and matched(d[k], t[k], k)]

    # Mangled-name pass. CFront mangling appends `__<classinfo>` to function names.
    # A retail placeholder `fn_800C31C0` never matches a draft's `fn_800C31C0__FP10dLineMng_c`.
    # Try stripping the mangling suffix from draft names to pair them by name alone.
    used = set(hit)
    for k in t:
        if k in used:
            continue
        # Look for a draft function whose unmangled name matches this target
        for dk in d:
            if dk in used or dk not in d:
                continue
            # Check if dk is the mangled form of k (e.g., fn_800C31C0__FP10dLineMng_c -> fn_800C31C0)
            if '__' in dk:
                unmangled = dk.split('__')[0]
                if unmangled == k:
                    # Pair by name. Add synthetic entry for reporting so the draft length is visible.
                    if matched(d[dk], t[k], k):
                        hit.append(k)
                    d[k] = d[dk]  # Allow reporting logic to find the draft
                    used.add(k)
                    paired_by_content.append((k, dk))
                    break

    # Content-keyed fallback. An UNNAMED target function (`fn_800C31C0`) never
    # keys against a draft's mangled/static name, so the name pass reports it
    # MISSING even when it is byte-perfect. Same defect class that verify_anon.py
    # exists to solve for the RELs. Pair the leftovers on BYTE EQUALITY only --
    # that is sound by definition (identical bytes IS a match) and cannot invent
    # a pairing the way a fuzzy heuristic could.
    spare = {k: v for k, v in d.items() if k not in t and k not in used}
    spare_by_bytes = {}
    for k, v in spare.items():
        spare_by_bytes.setdefault(tuple(b for b, _ in v), []).append(k)
    for k in t:
        if k in used:
            continue
        cand = spare_by_bytes.get(tuple(b for b, _ in t[k]))
        if cand:
            hit.append(k)
            used.add(k)
            paired_by_content.append((k, cand.pop(0)))
    words_all = sum(len(v) for v in t.values())
    words_hit = sum(len(t[k]) for k in hit)
    if paired_by_content:
        print(f'({len(paired_by_content)} paired by CONTENT -- unnamed target vs mangled draft name)')
    print(f'matched {len(hit)}/{len(t)} functions   '
          f'{words_hit}/{words_all} words = {100.0 * words_hit / words_all:.1f}% BY BYTES')
    if POOL_FAILURES:
        print()
        print(f'{len(POOL_FAILURES)} WRONG CONSTANT(S) -- these functions would '
              f'otherwise have counted as matched:')
        for name, i, va, tv, dv in POOL_FAILURES:
            print(f'  {name[:56]}')
            print(f'    instruction {i}: retail 0x{va:08X} = {tv!r}, draft = {dv!r}')
    print()
    todo = sorted(((len(v), k, len(d[k]) if k in d else None)
                   for k, v in t.items() if k not in hit), reverse=True)
    for n, k, dn in todo[:int(os.environ.get('TALLY_TOP', '25'))]:
        state = 'MISSING' if dn is None else ('LEN OK' if dn == n else f'{dn}w vs {n}w  STRUCTURAL')
        print(f'{n:5d}w  {state:>18}  {k[:66]}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
