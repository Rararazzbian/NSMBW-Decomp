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


def matched(draft_fn, target_fn):
    """The union gate. Bytes first -- it is the real criterion."""
    if [b for b, _ in draft_fn] == [b for b, _ in target_fn]:
        return True
    return (harness.canonicalise([t for _, t in draft_fn])
            == harness.canonicalise([t for _, t in target_fn]))


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

    # Name-keyed pass.
    hit = [k for k in t if k in d and matched(d[k], t[k])]

    # Content-keyed fallback. An UNNAMED target function (`fn_800C31C0`) never
    # keys against a draft's mangled/static name, so the name pass reports it
    # MISSING even when it is byte-perfect. Same defect class that verify_anon.py
    # exists to solve for the RELs. Pair the leftovers on BYTE EQUALITY only --
    # that is sound by definition (identical bytes IS a match) and cannot invent
    # a pairing the way a fuzzy heuristic could.
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
            hit.append(k)
            used.add(k)
            paired_by_content.append((k, cand.pop(0)))
    words_all = sum(len(v) for v in t.values())
    words_hit = sum(len(t[k]) for k in hit)
    if paired_by_content:
        print(f'({len(paired_by_content)} paired by CONTENT -- unnamed target vs mangled draft name)')
    print(f'matched {len(hit)}/{len(t)} functions   '
          f'{words_hit}/{words_all} words = {100.0 * words_hit / words_all:.1f}% BY BYTES')
    print()
    todo = sorted(((len(v), k, len(d[k]) if k in d else None)
                   for k, v in t.items() if k not in hit), reverse=True)
    for n, k, dn in todo[:25]:
        state = 'MISSING' if dn is None else ('LEN OK' if dn == n else f'{dn}w vs {n}w  STRUCTURAL')
        print(f'{n:5d}w  {state:>18}  {k[:66]}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
