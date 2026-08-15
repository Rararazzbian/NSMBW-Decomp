"""Check a draft's VTABLE SLOT ASSIGNMENT against the target's.

Why this exists
---------------
`.text` byte-identity does not prove a unit correct, and this is the sharpest
case. An actor's `create`, `execute`, `draw` and `doDelete` frequently all
compile to `li r3, 1; blr` -- **identical bytes**. Any permutation of those four
across their vtable slots produces a byte-identical `.text`, so a per-function
diff reports a clean 10/10 for a class that is semantically scrambled. The only
evidence is the vtable, and the failure surfaces at link time as a handful of
differing relocation bytes.

This bit three units in one day:

  d_a_wm_grid.cpp        doDelete sat where the original has execute
  d_a_wm_kinoko_1up.cpp  vf80/vf7C swapped (both a bare `blr`)
  d_a_wm_tower.cpp       checked clean by hand -- the reason it landed first try

How the comparison is possible at all
-------------------------------------
The target's own functions are anonymous (`fn_2_XXXXXX`), so the draft's slot
names can never be compared to the target's directly. Instead we pair draft
functions to target addresses by INSTRUCTION CONTENT -- the same pairing
`verify_anon.py` does -- and then ask, for each slot, whether the draft's entry
is the function that lives at the address the target's slot points to.

Inherited slots (`preCreate__10dWmActor_cFv` and friends) carry real names in
both, so those are compared by name. A mismatch there means the class derives
from the wrong base, which is worth catching too.

Usage
-----
    python wip/wm_units/check_vtable.py <draft.txt> <target_data.txt> \
        <target_vt_label> <lo> <hi> <target_text.o> [...]

e.g.
    python wip/wm_units/check_vtable.py draft.txt \
        bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.txt lbl_2_data_44CC0 \
        0x164230 0x164430 bin/dtkspl/d_basesNP/obj/auto_00_00164204_text.o
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', '..', 'tools', 'auto_decomp'))
import harness as H  # noqa: E402
from verify_anon import functions, norm  # noqa: E402


def vtable_entries(path, label):
    """The .4byte operands of a named .obj block, in order."""
    text = open(path, encoding='utf-8', errors='replace').read()
    m = re.search(r'^\.obj %s, \w+\n(.*?)^\.endobj' % re.escape(label),
                  text, re.M | re.S)
    if not m:
        raise SystemExit('no .obj %s in %s' % (label, path))
    return re.findall(r'^\s*\.4byte (\S+)\s*$', m.group(1), re.M)


def draft_vtable(path):
    text = open(path, encoding='utf-8', errors='replace').read()
    m = re.search(r'^\.obj (__vt__\S+?), \w+\n(.*?)^\.endobj', text, re.M | re.S)
    if not m:
        raise SystemExit('no __vt__ object in %s' % path)
    return m.group(1), re.findall(r'^\s*\.4byte (\S+)\s*$', m.group(2), re.M)


def main():
    if len(sys.argv) < 7:
        print(__doc__)
        return 1
    draft_txt, tdata, label = sys.argv[1], sys.argv[2], sys.argv[3]
    lo, hi = int(sys.argv[4], 0), int(sys.argv[5], 0)

    cache = os.path.join(os.path.dirname(os.path.abspath(__file__)), '_dis')
    os.makedirs(cache, exist_ok=True)
    target = []
    for obj in sys.argv[6:]:
        out = os.path.join(cache, os.path.basename(obj) + '.txt')
        if not os.path.exists(out):
            H.disasm(obj, out)
        target += functions(out, with_addr=True)
    target = sorted(x for x in target if lo <= x[0] < hi)

    # Pair draft functions to target addresses by PLACEMENT ORDER, not by
    # content lookup. Content alone cannot disambiguate two functions with
    # identical bodies -- which is the exact case this tool exists for -- and a
    # first-match-wins map sends all four of an actor's `li r3,1; blr` methods
    # to the lowest address, which falsely condemned the landed, 5/5-verified
    # d_a_wm_grid.cpp when I first wrote this.
    #
    # What actually determines a function's address is its order in the object:
    # the linker lays the object's .text down in order. So walk the targets in
    # ascending address and consume the first as-yet-unused draft whose body
    # matches -- drafts are in object order, so identical bodies are consumed in
    # the order they will be placed. This is the same greedy pairing
    # verify_anon.py reports, reused here for its assignment rather than its
    # count.
    drafts = functions(draft_txt)
    addr_of, used = {}, set()
    for addr, _tname, tins in target:
        want = norm(tins)
        for i, (dname, dins) in enumerate(drafts):
            if i not in used and norm(dins) == want:
                used.add(i)
                addr_of[dname] = addr
                break

    vtname, dents = draft_vtable(draft_txt)
    tents = vtable_entries(tdata, label)

    print('draft vtable: %s  (%d slots, %#x bytes)' % (vtname, len(dents) - 2, len(dents) * 4))
    print('target      : %s  (%d slots, %#x bytes)' % (label, len(tents) - 2, len(tents) * 4))
    if len(dents) != len(tents):
        print('\nSLOT COUNT DIFFERS by %d -- the class declares the wrong virtuals.'
              % (len(dents) - len(tents)))

    print('\n%-4s %-34s %-34s %s' % ('slot', 'target', 'draft', 'verdict'))
    bad = 0
    for i in range(max(len(dents), len(tents))):
        t = tents[i] if i < len(tents) else '(none)'
        d = dents[i] if i < len(dents) else '(none)'
        m = re.fullmatch(r'fn_2_([0-9A-Fa-f]+)', t)
        if m:
            want = int(m.group(1), 16)
            got = addr_of.get(d)
            if got == want:
                verdict = 'ok'
            elif not (lo <= want < hi):
                # The slot points outside this unit, so it is a method inherited
                # from a base class living in another TU. Nothing to check here,
                # and it is not a defect -- kinoko_1up inherits create/execute/
                # draw/doDelete from daWmKinokoBase_c at 0x16B470+.
                verdict = 'skip (inherited from another TU at %#x)' % want
            elif got is None:
                verdict = ('unverifiable -- no draft fn matches the target at %#x '
                           '(that function is still differing)' % want)
            else:
                verdict = 'WRONG SLOT -- this draft fn is %#x, slot wants %#x' % (got, want)
                bad += 1
        elif t == d:
            verdict = 'ok'
        elif t == '(none)' or d == '(none)':
            verdict = 'MISSING SLOT'
            bad += 1
        else:
            verdict = 'DIFFERENT SYMBOL -- wrong base class?'
            bad += 1
        if verdict == 'ok':
            continue
        print('%-4d %-34s %-34s %s' % (i, t[:34], d[:34], verdict))

    print('\n%s' % ('VTABLE CLEAN' if not bad else '%d slot(s) wrong' % bad))
    return 0 if not bad else 1


if __name__ == '__main__':
    raise SystemExit(main())
