"""Name the class behind a REL `.bss` singleton pointer, and read its `sizeof`.

Follows on from `bss_classify.py`, which identifies WHICH labels are singleton
instance pointers (exactly two writes: create and destroy). This resolves WHAT
they point at, mechanically:

  * `sizeof` comes straight off the allocation -- `li rN, SIZE; bl <operator new>`
    in the creating function. Not inferred from field offsets, not guessed from a
    layout: read out of the instruction that allocates the object.
  * the base class and member types come from the `bl` targets between the
    allocation and the pointer store, resolved through
    `bin/dtk/wiimj2d_symbols.txt`. **That is the full DOL symbol map. `syms.txt`
    is only our small curated list and does NOT contain them** -- looking there,
    finding nothing, and concluding "unidentified" is exactly how `lbl_2_bss_11B70`
    stayed a mystery through an otherwise exhaustive search.
  * the owning unit comes from `profile_map.py`, by locating the write site in a
    profile's `.text` range.

What this tool does NOT prove
-----------------------------
It does not verify that the pointer stored is the one `operator new` returned.
When the allocation sits far from the store, the allocation it found may belong
to a MEMBER or a temporary instead, and the reported `sizeof` would then be that
object's size, not the singleton's. The tool warns when the span is wide; treat
those results as candidates until the dataflow is confirmed by eye.

Usage
-----
    python resolve_singleton.py d_basesNP 0x11B70
"""

import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from profile_map import relocations, profiles

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TEXT_FILE_OFFSET = 0xF0
# Both spellings appear: fBase_c has its own operator new, some units use the global.
OPERATOR_NEW = ("__nw__7fBase_cFUl", "__nw__FUl")
WINDOW = 0x300      # how far back from the store to look for the allocation
NEAR = 0x120        # an allocation further than this from the store is suspect


def dol_symbols():
    path = os.path.join(ROOT, "bin", "dtk", "wiimj2d_symbols.txt")
    table = {}
    pattern = re.compile(r"^(\S+)\s*=\s*\.\w+:0x([0-9A-Fa-f]+)")
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        for line in handle:
            match = pattern.match(line.strip())
            if match:
                table[int(match.group(2), 16)] = match.group(1)
    return table


def unit_ranges(module, rel):
    """[(text_start, profile_name)] sorted, from each profile's classInit."""
    units = []
    for data_addr, name in profiles(module):
        entry = rel.get((5, data_addr))
        if entry and entry[0] == 1:
            units.append((entry[1], name))
    return sorted(units)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1
    module, target = sys.argv[1], int(sys.argv[2], 0)

    blob = open(os.path.join(ROOT, "bin", "%s.rel" % module), "rb").read()
    rel = relocations(module)
    names = dol_symbols()

    def word(addr):
        off = TEXT_FILE_OFFSET + addr
        return struct.unpack(">I", blob[off:off + 4])[0]

    def call_target(addr):
        if (word(addr) >> 26) != 18:
            return None
        entry = rel.get((1, addr + 2)) or rel.get((1, addr))
        return names.get(entry[1]) if entry else None

    writes = []
    for (patched_section, patched_addr), (tgt_section, addend) in rel.items():
        if tgt_section == 6 and patched_section == 1 and addend == target:
            addr = patched_addr & ~3
            if (word(addr) >> 26) == 36:
                writes.append(addr)
    writes.sort()
    if len(writes) != 2:
        print("Not a two-write singleton: %d write(s) found." % len(writes))
        return 1

    # WHICH write is the create is NOT decided by address order. WM_KOOPASHIP has
    # its destroy at the LOWER address, and assuming otherwise made this tool
    # hunt for an allocation backwards from the teardown -- where it found a
    # nearby unrelated `li r3, 0x40` and reported it as the singleton's sizeof.
    # The destroy is the one whose stored register is fed by `li rN, 0`.
    def stores_zero(addr):
        source = (word(addr) >> 21) & 31
        for back in range(addr - 4, addr - 0x18, -4):
            instruction = word(back)
            if (instruction >> 26) == 14 and ((instruction >> 21) & 31) == source:
                return ((instruction >> 16) & 31) == 0 and (instruction & 0xFFFF) == 0
        return False

    zeroing = [addr for addr in writes if stores_zero(addr)]
    if len(zeroing) == 1:
        destroy = zeroing[0]
        create = [addr for addr in writes if addr != destroy][0]
    else:
        create, destroy = writes
        print("  (could not tell create from destroy by the stored value;")
        print("   falling back to address order, which may be wrong)")
    print("lbl_2_bss_%X  --  create 0x%X, destroy 0x%X" % (target, create, destroy))
    print("")

    for index, (start, name) in enumerate(units := unit_ranges(module, rel)):
        end = units[index + 1][0] if index + 1 < len(units) else 1 << 32
        if start <= create < end:
            print("  owning profile   %s   (.text 0x%X-0x%X)" % (name, start, end))
            break

    # Prove the pointer being stored is the one `operator new` returned, rather
    # than assuming it because an allocation happens to sit nearby. The result
    # arrives in r3 and is typically carried through one or more `mr` moves (and
    # through a constructor, which returns `this` in r3) before the store.
    def carries_new_result(alloc_addr, store_addr):
        live = {3}
        for addr in range(alloc_addr + 4, store_addr + 4, 4):
            instruction = word(addr)
            opcode = instruction >> 26
            if opcode == 31 and ((instruction >> 1) & 0x3FF) == 444:   # mr rA, rS
                source = (instruction >> 21) & 31
                dest = (instruction >> 16) & 31
                back = (instruction >> 11) & 31
                if source == back:
                    if source in live:
                        live.add(dest)
                    else:
                        live.discard(dest)
            elif opcode == 18 and (instruction & 1):                   # bl: r3 = this
                live.add(3)
        return ((word(store_addr) >> 21) & 31) in live

    alloc = None
    size = None
    verified = False
    for addr in range(create, create - WINDOW, -4):
        if call_target(addr) in OPERATOR_NEW:
            for back in range(addr - 4, addr - 0x48, -4):
                instruction = word(back)
                if (instruction >> 26) == 14 and ((instruction >> 16) & 31) == 0:
                    size = instruction & 0xFFFF
                    print("  sizeof           0x%X   (li r%d, 0x%X at 0x%X; bl %s)"
                          % (size, (instruction >> 21) & 31, size, back,
                             call_target(addr)))
                    break
            alloc = addr
            verified = carries_new_result(addr, create)
            break

    if alloc is None:
        print("  allocation not located within 0x%X bytes before the store." % WINDOW)
        print("  Skipping the call list: printing a fixed window would name the")
        print("  PRECEDING function's callees as if they belonged to this class.")
        return 0

    if verified:
        print("  dataflow         VERIFIED -- the stored pointer is the value")
        print("                   operator new returned (traced through mr/bl).")
    else:
        print("  dataflow         NOT VERIFIED -- the register stored is not the")
        print("                   one this allocation produced. The sizeof above")
        print("                   probably belongs to a MEMBER or a temporary,")
        print("                   not to the singleton. Treat it as unknown.")

    span = create - alloc
    if span > NEAR and not verified:
        print("")
        print("  WARNING: the allocation is 0x%X bytes before the store." % span)
        print("  It may allocate a DIFFERENT object (a member, a temporary) rather")
        print("  than the singleton, in which case the sizeof above is that object's.")
        print("  Treat it as a CANDIDATE until you confirm the stored register is")
        print("  the one operator new returned.")

    print("")
    print("  calls between the allocation and the pointer store")
    print("  (base class first, then members constructed in place):")
    seen = set()
    for addr in range(alloc, create + 0x40, 4):
        name = call_target(addr)
        if name and name not in seen:
            seen.add(name)
            print("    0x%X  %s" % (addr, name))
    return 0


if __name__ == "__main__":
    sys.exit(main())
