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
        """Does the register stored actually hold what `operator new` returned?

        Two things make the naive version unsound, and both produced a WRONG
        row in BSS_SINGLETONS.md before they were fixed:

        1. **A function boundary between the allocation and the store.** Register
           liveness does not survive one. MINI_GAME_GUN_BATTERY's classInit
           allocates 0xF4 bytes and RETURNS (`blr`); the singleton store lives in
           a different function entirely, which allocates nothing.
        2. **Any instruction that redefines the tracked register kills it.**
           Tracking only `mr` moves misses `li`/`lis`/`lwz` overwrites, so a
           register reloaded for an unrelated call still looked live. In that same
           unit `li r3, 0x164` then `bl createChild` meant the stored value was a
           CHILD OBJECT, not the allocation -- yet r3 was reported as carrying it.

        A `bl` re-establishes r3 only under the constructor convention (returns
        `this`), which holds only if r3 was live going in.
        """
        live = {3}
        for addr in range(alloc_addr + 4, store_addr + 4, 4):
            instruction = word(addr)
            opcode = instruction >> 26

            # Function boundary: liveness cannot cross it.
            if instruction == 0x4E800020 or opcode == 37:   # blr, stwu (prologue)
                return False

            if opcode == 31 and ((instruction >> 1) & 0x3FF) == 444:   # mr rA, rS
                source = (instruction >> 21) & 31
                dest = (instruction >> 16) & 31
                if source == ((instruction >> 11) & 31):
                    live.add(dest) if source in live else live.discard(dest)
                    continue

            if opcode == 18 and (instruction & 1):          # bl
                # Constructor convention returns `this` in r3 -- but only if r3
                # actually held the object going in.
                if 3 not in live:
                    live.discard(3)
                continue

            # Anything else that writes a register kills our tracking of it.
            if opcode in (14, 15, 24, 25, 32, 34, 40, 48, 50, 33, 35):
                live.discard((instruction >> 21) & 31)
            elif opcode == 31:
                live.discard((instruction >> 16) & 31)

        return ((word(store_addr) >> 21) & 31) in live

    def function_start(addr):
        """Walk back to the prologue of the function containing `addr`."""
        for probe in range(addr, max(0, addr - 0x2000), -4):
            if word(probe) >> 26 == 37:          # stwu r1, -N(r1)
                return probe
        return None

    def find_alloc(lo, hi):
        """The `bl <operator new>` in [lo, hi), with its size immediate."""
        for addr in range(hi, lo, -4):
            if call_target(addr) in OPERATOR_NEW:
                for back in range(addr - 4, max(lo, addr - 0x48), -4):
                    instruction = word(back)
                    if (instruction >> 26) == 14 and ((instruction >> 16) & 31) == 0:
                        return addr, instruction & 0xFFFF
                return addr, None
        return None, None

    # An allocation only tells you the singleton's size if it is the value being
    # stored. Scope the search to the STORING FUNCTION -- a fixed byte window
    # silently crosses `blr` into an unrelated function, and doing so put a wrong
    # sizeof in BSS_SINGLETONS.md: MINI_GAME_GUN_BATTERY's classInit allocates
    # 0xF4 and returns, while the store lives in another function that allocates
    # nothing and stores `createChild()`'s result instead.
    store_fn = function_start(create)
    alloc = size = None
    verified = False
    if store_fn is not None:
        alloc, size = find_alloc(store_fn, create)
        if alloc is not None:
            verified = carries_new_result(alloc, create)
            print("  sizeof           0x%X   (allocated in the storing function"
                  % size if size else "  sizeof           unknown")
            if size:
                print("                   at 0x%X, and the stored register traces"
                      % alloc)
                print("                   to it)" if verified else
                      "                   to something ELSE -- sizeof NOT this object's)")

    if alloc is None:
        print("  sizeof           NOT DETERMINED from this site.")
        print("                   No `operator new` in the function that stores the")
        print("                   pointer, so the object is allocated elsewhere and")
        print("                   arrives as an argument. Report the caller's")
        print("                   allocation only as INFERRED, never as measured:")
        callers = sorted({addr for addr in range(max(0, store_fn - 0x8000), store_fn)
                          if (word(addr) >> 26) == 18 and (word(addr) & 1)
                          and (addr + (((word(addr) & 0x03FFFFFC) ^ 0x02000000) - 0x02000000)) == store_fn}) if store_fn else []
        for caller in callers[:3]:
            caller_fn = function_start(caller)
            if caller_fn is None:
                continue
            caller_alloc, caller_size = find_alloc(caller_fn, caller)
            if caller_size:
                print("                     caller 0x%X allocates 0x%X (INFERRED)"
                      % (caller_fn, caller_size))
        if not callers:
            print("                     (no direct caller found nearby)")
        return 0

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
