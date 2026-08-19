"""Classify what a REL `.bss` label actually IS, by how the code touches it.

Why this exists
---------------
`lbl_2_bss_11B70` was recorded as "a shared singleton whose type is genuinely
UNIDENTIFIED" after an exhaustive search: 130+ references found, three candidate
classes checked against its apparent field offsets, all of `include/` grepped for
those offsets. Every step was competent and the whole search was doomed, because
it rested on a false premise -- **that the .bss label was the object.**

It was a 4-byte POINTER. The "field offsets" were displacements off a value
LOADED from that cell, so they describe a heap object the label merely points at.
No class in `include/` could ever have matched.

The tell is visible in four commands, and it is mechanical:

    lis  r3, lbl@ha
    lwz  r5, lbl@l(r3)     <- op 32: the cell holds a POINTER
    stb  r4, 0x544(r5)     <- the offset belongs to the POINTED-TO object

versus a label that IS the object, where the patched instruction is the access
itself, or an `addi` taking its address.

So: **classify a label's references by the OPCODE of the patched instruction
before theorising about its type.** The read/write census alone names the shape:

    2 writes, many reads   -> a singleton instance pointer (create + destroy)
    many writes            -> mutable global state
    `addi` only            -> the address is taken; the label IS the object
    `lwz`/`stw` at the site -> direct access; the label IS the object

Having found a singleton pointer, the two write sites are worth everything: the
creating function allocates with `li r3, SIZE; bl __nw__7fBase_cFUl`, giving
`sizeof` exactly, and its `bl` targets resolve the base class and members.
Resolve those through `bin/dtk/wiimj2d_symbols.txt` -- the full DOL map -- and
NOT through `syms.txt`, which is only our small curated list.

Usage
-----
    python bss_classify.py d_basesNP 0x11B70
    python bss_classify.py d_basesNP            # census of every referenced label
"""

import collections
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from profile_map import relocations

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TEXT_SECTION = 1
BSS_SECTION = 6
TEXT_FILE_OFFSET = 0xF0

# Opcodes we care about, and what each implies about the label.
LOAD_OPS = {32: "lwz", 34: "lbz", 40: "lhz", 48: "lfs", 50: "lfd"}
STORE_OPS = {36: "stw", 38: "stb", 44: "sth", 52: "stfs", 54: "stfd"}
ADDR_OPS = {14: "addi", 15: "lis", 24: "ori"}


def text_words(module):
    path = os.path.join(ROOT, "bin", "%s.rel" % module)
    with open(path, "rb") as f:
        return f.read()


def opcode_at(blob, addr):
    off = TEXT_FILE_OFFSET + (addr & ~3)
    word = struct.unpack(">I", blob[off:off + 4])[0]
    return word >> 26, word


def is_dereferenced(blob, site):
    """Is the value LOADED from this label then used as a base register?

    This is what separates a singleton instance pointer from a plain int that
    merely happens to have two writers. A pointer's loaded value shows up as the
    rA of a following load/store (`stb r4, 0x544(r5)`); a counter's loaded value
    is only compared or arithmetic'd (`addi r0, r3, -1`).

    Two writes alone is NOT sufficient -- it produced false positives on a
    BIGHANA_MGR counter (decrement) and an OBJ_WENDY state variable (compare and
    assign), both of which would otherwise have sent someone hunting for a class
    that does not exist.
    """
    addr = site & ~3
    instruction = struct.unpack(">I", blob[TEXT_FILE_OFFSET + addr:TEXT_FILE_OFFSET + addr + 4])[0]
    if (instruction >> 26) not in LOAD_OPS:
        return False
    loaded = (instruction >> 21) & 31
    for offset in range(4, 0x40, 4):
        following = struct.unpack(
            ">I",
            blob[TEXT_FILE_OFFSET + addr + offset:TEXT_FILE_OFFSET + addr + offset + 4],
        )[0]
        opcode = following >> 26
        if opcode in LOAD_OPS or opcode in STORE_OPS:
            if ((following >> 16) & 31) == loaded:
                return True
        # The register was overwritten; stop following it.
        if opcode in LOAD_OPS and ((following >> 21) & 31) == loaded:
            return False
    return False


def classify(module, target=None):
    rel = relocations(module)
    blob = text_words(module)

    labels = collections.defaultdict(list)
    for (patched_section, patched_addr), (tgt_section, addend) in rel.items():
        if tgt_section != BSS_SECTION or patched_section != TEXT_SECTION:
            continue
        labels[addend].append(patched_addr)

    if target is not None:
        selected = [(target, labels.get(target, []))]
    else:
        selected = sorted(labels.items(), key=lambda kv: -len(kv[1]))[:40]

    for addend, sites in selected:
        census = collections.Counter()
        stores = []
        for site in sorted(sites):
            op, _ = opcode_at(blob, site)
            if op in LOAD_OPS:
                census["load (%s)" % LOAD_OPS[op]] += 1
            elif op in STORE_OPS:
                census["store (%s)" % STORE_OPS[op]] += 1
                stores.append(site & ~3)
            elif op in ADDR_OPS:
                census["address (%s)" % ADDR_OPS[op]] += 1
            else:
                census["op %d" % op] += 1

        loads = sum(v for k, v in census.items() if k.startswith("load"))
        writes = sum(v for k, v in census.items() if k.startswith("store"))
        dereferenced = any(is_dereferenced(blob, site) for site in sorted(sites))

        print("lbl_%s_bss_%X  --  %d relocations" % (module.split("_")[0][-1] or "2", addend, len(sites)))
        for kind, count in census.most_common():
            print("    %-18s %d" % (kind, count))

        if writes == 2 and loads and dereferenced:
            verdict = ("SINGLETON INSTANCE POINTER -- exactly two writes is the "
                       "create/destroy pair.\n    Disassemble the write sites: the "
                       "creator's `li rN, SIZE; bl __nw__7fBase_cFUl` gives sizeof,\n"
                       "    and its `bl` targets (resolve via bin/dtk/wiimj2d_symbols.txt)\n"
                       "    give the base class and the member types.")
        elif writes == 2 and loads:
            verdict = (
                "PLAIN VALUE (int / counter / state), NOT a singleton pointer."
                "\n    Two writes alone does NOT mean a singleton. The value loaded"
                "\n    from this label is never used as a BASE REGISTER, only compared"
                "\n    or arithmetic'd -- so nothing is being pointed at, and there is"
                "\n    no class behind it to find."
            )
        elif writes > 2:
            verdict = "MUTABLE GLOBAL STATE -- many writers; the label likely IS the object."
        elif writes == 0 and loads:
            verdict = "READ-ONLY -- written elsewhere, or set once by non-relocated code."
        else:
            verdict = "ADDRESS TAKEN -- the label is very likely the object itself."
        print("    => %s" % verdict)
        if stores:
            print("    write sites: %s" % ", ".join("0x%X" % s for s in stores))
        print()


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    module = sys.argv[1]
    target = int(sys.argv[2], 0) if len(sys.argv) > 2 else None
    classify(module, target)
    return 0


if __name__ == "__main__":
    sys.exit(main())
