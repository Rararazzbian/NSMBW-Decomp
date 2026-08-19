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

WHAT SYMBOL NORMALISATION COSTS YOU -- READ THIS BEFORE TRUSTING AN N/N
-----------------------------------------------------------------------
Normalising the relocation symbol is REQUIRED (see above) and it has a price:
**a reference to the WRONG pool entry compares byte-identical.**
`lfs f1, lbl_2_rodata_8884@l(r3)` and `lfs f1, lbl_2_rodata_88A0@l(r3)` both
reduce to `lfs f1, SYM@l(r3)`.

This is not hypothetical. `d_a_wm_ghost.cpp` read a clean 13/13 while its
`create()` passed `0.0f` to `mClipSphere.set()` where the original passes
`180.0f` -- a real semantic difference, invisible here, invisible to
`check_sections.py` (the pool was the right SIZE) and invisible to
`check_vtable.py`. It surfaced only in the LINKED binary, as two bytes in the
REL's relocation table pointing at `.rodata+0x20` instead of `+0x4`.

So an N/N from this tool means "every instruction matches modulo which symbol it
names". It does NOT mean the unit loads the right constants. When a unit reads
clean and the link still fails, diff the built REL against `original/` and
decode the differing relocation entries -- that is the only view that shows
which pool entry an instruction actually reaches.

LIMIT OF THE PAIRING, AND OF THE ORDER CHECK BUILT ON IT
--------------------------------------------------------
The pairing is greedy on instruction content, so **two functions with identical
bodies can be paired to each other's targets**. That is the same "two functions,
one body" trap `check_vtable.py` exists for, and it reaches this tool too.

Real case: `d_a_wm_sandpillar.cpp`'s `__dt__Q23mEf8effect_cFv` was paired with
target `fn_2_179290`, which on inspection is `sStateID_c`'s scalar deleting
destructor -- a different class entirely. Deleting-destructor wrappers (null
check, one member-dtor call, optional `__dl__`) are byte-identical in shape
across unrelated classes.

Consequence: a reported `FUNCTION ORDER IS WRONG` can be an artefact of a
mis-pairing rather than a real ordering defect. **Before acting on an order
violation, read the target function at that address and confirm it is what the
tool says it is.**

PARTIALLY FIXED: the pairing no longer takes the first matching candidate
unconditionally. Among equally-valid candidates it prefers one that keeps the
matched sequence ascending, which removes the whole class of order violations
invented by a tie between byte-identical bodies (WM_ANCHOR's two lone-`blr` weak
symbols were reported as an ordering defect for exactly that reason). This is
sound rather than a papering-over: byte-identical functions emit identical
`.text`, so if an ascending assignment exists the object is consistent with
correct definition order. A REAL inversion admits no ascending assignment and is
still reported.

It does NOT fix the other half of the trap -- a draft function paired to a
target that is genuinely a different function of the same shape (the sandpillar
case above). Confirming the target at that address is still required.
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
        # Two alternatives, and the QUOTED one must come first. MWCC quotes any
        # symbol it cannot spell bare -- notably template instantiations like
        # "__vt__32sFStateFct_c<daWmSandPillar_c,sStateMethodUsr_FI_c>". The
        # unquoted pattern below uses [^\s,]*, which stops dead at the comma
        # inside such a name, so the symbol never normalised and every function
        # referencing a template vtable reported phantom differences.
        # d_a_wm_sandpillar.cpp's constructor showed 4 that way, all fictitious.
        line = re.sub(r'"[^"]*"@(ha|l|sda21|sda2)\b', r'SYM@\1', line)
        line = re.sub(r'[.A-Za-z_@$][^\s,]*@(ha|l|sda21|sda2)\b', r'SYM@\1', line)
        line = re.sub(r'^(bl|b) \S+$', r'\1 SYM', line)
        line = re.sub(r'\.L_[0-9A-Fa-f]+', 'LBL', line)
        out.append(line)
    return out


# dtk ENDS A FUNCTION AT AN UNCONDITIONAL BRANCH. When MWCC emits an
# unreachable trailing `blr` after a tail call, dtk splits that `blr` off as its
# own 4-byte "function" -- so a draft that correctly emits it reads as ONE
# INSTRUCTION LONGER than the target.
#
# This cost a full agent round on d_a_wm_sandpillar.cpp: `executeState_BottomWait`
# and `executeState_TopWait` each reported "1 differing -- an extra trailing
# blr", and ten source reformulations were measured against a defect that did
# not exist. The target's own bytes settle it -- `fn_2_1780C0` is 0x34 and ends
# `4E800420` (bctr); the very next word at `0x1780F4` is `4E800020` (blr), which
# dtk lists as `fn_2_1780F4`, size 0x4. The draft was byte-identical throughout.
#
# The fix belongs at the COMPARISON, not the parse. Restitching the target's
# function list was tried twice and cascades: merging after `b`/`blr` as well
# took a correct 64/66 to 42/52, and merging after `bctr` alone still swallowed
# four legitimate functions and invented two new differences. Whether the `blr`
# belongs to the previous function depends on whether the DRAFT emitted one, and
# only the comparison knows that.
def eq_mod_tail_blr(target, draft):
    """True if `draft` equals `target`, or is `target` plus one dead `blr`.

    Allowed ONLY when the target's last instruction is `bctr`: after a tail call
    a `blr` is unreachable, so it cannot be a real function of its own and its
    presence or absence changes nothing that executes.
    """
    if draft == target:
        return True
    return (len(draft) == len(target) + 1
            and draft[:-1] == target
            and draft[-1].strip() == 'blr'
            and target and target[-1].strip() == 'bctr')


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
    last = -1
    for addr, name, ins in target:
        want = norm(ins)
        # Collect EVERY unused candidate, not just the first. Two functions with
        # byte-identical bodies are interchangeable candidates, and taking the
        # first one unconditionally invents ordering violations out of a tie:
        # WM_ANCHOR emits `finalUpdate__12dBaseActor_cFv` and
        # `vf74__12daWmAnchor_cFv` as separate weak symbols whose bodies are both
        # a lone `blr`, and greedy-first paired them the wrong way round and then
        # reported FUNCTION ORDER IS WRONG on the strength of it.
        candidates = [i for i, (dname, dins) in enumerate(drafts)
                      if i not in used and eq_mod_tail_blr(want, norm(dins))]
        # Prefer a candidate that keeps the matched sequence ascending. This is
        # not cosmetic: if two draft functions have byte-identical bodies then
        # swapping them emits identical `.text`, so when an ascending assignment
        # EXISTS the emitted code is consistent with correct definition order and
        # there is no defect to report. A genuine inversion has no such
        # assignment and still reports.
        ascending = [i for i in candidates if i > last]
        hit = ascending[0] if ascending else (candidates[0] if candidates else None)
        if hit is not None:
            last = hit
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
