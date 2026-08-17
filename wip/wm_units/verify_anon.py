"""Verify a draft against a target whose functions are ANONYMOUS in the symbol map.

Why this exists
---------------
`harness.diff_fn` matches functions by name. That works when the target's
symbols are named. It does NOT work for units like `d_a_wm_grid.cpp` or
`d_a_wm_tower.cpp`, where every target function is `fn_2_XXXXXX` and the draft
emits real mangled names -- there is no common key, so a name-based diff
silently reports nothing and a per-function MATCH table built on it means
nothing.

Two normalisations are REQUIRED and both are legitimate, not fudges:

1. **Symbol names in relocations.** The target says
   `lis r4, lbl_2_data_44CC0@ha`; a correct draft says
   `lis r4, __vt__10daWmGrid_c@ha`. Same instruction, same address, different
   name for a symbol that has no name in the map. The linker resolves by
   address, so this is not a difference. Compare modulo the identifier.
2. **Local branch labels.** `.L_8016F3C0` versus `.L_00000123` is a naming
   artefact of where the disassembly started.

What is NOT normalised, because these are real differences: opcodes, register
numbers, immediates, and offsets. Register allocation is precisely the thing
that has blocked every unit on this project, so it must never be masked.

Usage
-----
    python wip/wm_units/verify_anon.py <draft.txt> <lo> <hi> <target.o> [target.o ...]

Prints, per target function in [lo, hi): its address, its size, and either the
draft function that matches it or the number of differing instructions against
its closest candidate.

A name after `MATCH <-` is a real pairing. A name after `differing vs ~` is only
the closest remaining draft function BY SIZE and is frequently not the target's
actual identity -- do not read it as one, and do not pass it on as one.
"""
import os
import re
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'tools', 'auto_decomp'))
import harness as H  # noqa: E402


def norm(instructions):
    out = []
    for line in instructions:
        # NOTE the leading '.' in the character class. MWCC names anonymous pool
        # objects '...rodata.0', '...data.3' and so on -- they start with DOTS.
        # An earlier version of this pattern did not accept them, so they never
        # normalised and every __sinit reported 2 spurious differences. This
        # tool then under-reported matches on exactly the functions it exists to
        # check, and I corrected a peer's results with it twice before noticing.
        line = re.sub(r'"?[.A-Za-z_@$][^\s,]*"?@(ha|l|sda21|sda2)\b', r'SYM@\1', line)
        line = re.sub(r'^(bl|b) \S+$', r'\1 SYM', line)
        line = re.sub(r'\.L_[0-9A-Fa-f]+', 'LBL', line)
        out.append(line)
    return out


def functions(path, with_addr=False):
    text = open(path, encoding='utf-8', errors='replace').read()
    out = []
    for m in re.finditer(r'^\.fn (\S+?), \w+\n(.*?)^\.endfn', text, re.M | re.S):
        body = m.group(2)
        ins = [x.group(1) for x in re.finditer(r'\*/\s*(.+?)\s*$', body, re.M)]
        if m.group(1).startswith(('gap_', 'pad_')):
            continue
        if with_addr:
            addrs = re.findall(r'/\* ([0-9A-Fa-f]{8}) ', body)
            if not addrs:
                continue
            out.append((int(addrs[0], 16), m.group(1), ins))
        else:
            out.append((m.group(1), ins))
    return out


def main():
    if len(sys.argv) < 5:
        print(__doc__)
        return 1
    draft, lo, hi = sys.argv[1], int(sys.argv[2], 0), int(sys.argv[3], 0)

    # Disassemble into a scratch dir, never next to the target objects -- those
    # live in the build tree and must not accumulate artefacts.
    cache = os.path.join(os.path.dirname(os.path.abspath(__file__)), '_dis')
    os.makedirs(cache, exist_ok=True)
    target = []
    for obj in sys.argv[4:]:
        out = os.path.join(cache, os.path.basename(obj) + '.txt')
        # Write via a PID-unique temp name and rename. Several agents share this
        # cache dir, and one of them clearing or half-writing an entry while
        # another reads it silently produced a wrong lookup.
        if not os.path.exists(out) or os.path.getsize(out) == 0:
            tmp = out + '.%d.tmp' % os.getpid()
            H.disasm(obj, tmp)
            try:
                os.replace(tmp, out)
            except OSError:
                pass
        target += functions(out, with_addr=True)
    target = sorted(x for x in target if lo <= x[0] < hi)

    drafts = functions(draft)
    used, exact, matched = set(), 0, []
    print('%-10s %-22s %5s  %s' % ('addr', 'target', 'size', 'result'))
    for addr, name, ins in target:
        want = norm(ins)
        hit = None
        for i, (dname, dins) in enumerate(drafts):
            if i not in used and norm(dins) == want:
                hit = i
                break
        matched.append(hit)
        if hit is not None:
            used.add(hit)
            exact += 1
            print('%#010x %-22s %5d  MATCH  <- %s' % (addr, name, len(ins), drafts[hit][0]))
        else:
            best, bestn = None, 10 ** 9
            for i, (dname, dins) in enumerate(drafts):
                if i in used:
                    continue
                a, b = norm(dins), want
                n = sum(1 for j in range(max(len(a), len(b)))
                        if (a[j] if j < len(a) else None) != (b[j] if j < len(b) else None))
                if n < bestn:
                    best, bestn = dname, n
            # NOTE: `best` is the CLOSEST REMAINING draft function by instruction
            # count, not an identification. When a target function matches
            # nothing, the nearest candidate is often an unrelated function that
            # happens to be a similar size -- course's 0x161840 was reported
            # "vs processCutsceneCommand" while actually being isWorld2SpecialType.
            # Labelled '~' so a reader cannot mistake a guess for a fact.
            print('%#010x %-22s %5d  %d differing vs ~%s' % (addr, name, len(ins), bestn, best))
    print('\n%d/%d byte-identical modulo symbol names' % (exact, len(target)))

    # ORDER CHECK. Matching every function proves nothing about their ORDER, and
    # the linker lays a unit's .text down in object order -- which is source
    # definition order. d_a_wm_smallcloud.cpp reported a clean 16/16 while
    # defining processCutsceneCommand before createModel where the original has
    # it after mode_exec; every function was byte-identical and the module still
    # failed, because every `bl` past that point had the wrong displacement.
    # The pairing above consumes drafts greedily in object order, so if the
    # matched draft indices are not ascending, the definition order is wrong.
    order = [i for i in matched if i is not None]
    if order != sorted(order):
        print('\nFUNCTION ORDER IS WRONG -- the draft defines these out of order.')
        print('The linker places .text in definition order, so this will not link')
        print('even at %d/%d. Target order:' % (exact, len(target)))
        prev = -1
        for addr, name, idx in zip([t[0] for t in target], [t[1] for t in target], matched):
            if idx is None:
                continue
            flag = '   <-- defined too late' if idx < prev else ''
            print('  %#010x  %s%s' % (addr, drafts[idx][0], flag))
            prev = max(prev, idx)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
