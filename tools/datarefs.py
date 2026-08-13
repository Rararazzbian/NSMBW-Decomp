#!/usr/bin/env python3
"""Collect every data address referenced by a .text address range of a DOL.

Scans PowerPC D-form instructions for:
  * lis/addis + addi/ori/load/store  (@ha / @l pairs)
  * r2-relative  (_SDA2_BASE_ -> .sdata2 / .sbss2)
  * r13-relative (_SDA_BASE_  -> .sdata  / .sbss)

Usage:  python datarefs.py <dol> <start_addr_hex> <end_addr_hex> [--sda BASE] [--sda2 BASE]

Written for the NSMBW decomp; used to bound a TU's data sections when the
lower neighbour is not decompiled and the gap cannot simply be subtracted.
"""
import struct
import sys

# D-form load/store opcodes: (opcode, mnemonic)
DFORM = {
    32: 'lwz', 33: 'lwzu', 34: 'lbz', 35: 'lbzu',
    36: 'stw', 37: 'stwu', 38: 'stb', 39: 'stbu',
    40: 'lhz', 41: 'lhzu', 42: 'lha', 43: 'lhau',
    44: 'sth', 45: 'sthu', 46: 'lmw', 47: 'stmw',
    48: 'lfs', 49: 'lfsu', 50: 'lfd', 51: 'lfdu',
    52: 'stfs', 53: 'stfsu', 54: 'stfd', 55: 'stfdu',
}

# Update-form loads/stores. These write the effective address back into rA, so
# a later instruction chained off rA computes from the updated base. Failing to
# model this is not a harmless approximation: one `lwzu r12, -0x1910(r5)` left
# r5's old base in place and made the following seven `lwz N(r5)` resolve to
# addresses ~0x1900 too high, i.e. into the NEXT translation unit's data. That
# reads as a real cross-unit reference and will send you deriving the wrong
# section bounds.
UPDATE = frozenset((33, 35, 37, 39, 41, 43, 45, 49, 51, 53, 55))


def load_dol(path):
    d = open(path, 'rb').read()
    off = struct.unpack('>18I', d[0x00:0x48])
    adr = struct.unpack('>18I', d[0x48:0x90])
    siz = struct.unpack('>18I', d[0x90:0xD8])
    secs = [(adr[i], siz[i], off[i]) for i in range(18) if siz[i]]
    return d, secs


def read(d, secs, addr, n):
    for a, s, o in secs:
        if a <= addr and addr + n <= a + s:
            return d[o + (addr - a): o + (addr - a) + n]
    return None


def signed16(v):
    return v - 0x10000 if v & 0x8000 else v


def scan(d, secs, start, end, sda, sda2):
    hi = {}          # reg -> (value<<16) from lis / addis
    refs = []        # (insn_addr, mnemonic, target, kind)
    a = start
    while a < end:
        w = read(d, secs, a, 4)
        if w is None:
            break
        w = struct.unpack('>I', w)[0]
        op = w >> 26
        rd = (w >> 21) & 31
        ra = (w >> 16) & 31
        imm = w & 0xFFFF

        if op == 15:                       # addis
            if ra == 0:
                hi[rd] = imm << 16
            elif ra in hi:
                hi[rd] = (hi[ra] + (signed16(imm) << 16)) & 0xFFFFFFFF
            else:
                hi.pop(rd, None)
        elif op in (14, 24) or op in DFORM:   # addi / ori / load / store
            mn = {14: 'addi', 24: 'ori'}.get(op, DFORM.get(op))
            resolved = None
            if ra == 2:
                resolved = (sda2 + signed16(imm)) & 0xFFFFFFFF
                refs.append((a, mn, resolved, 'r2'))
            elif ra == 13:
                resolved = (sda + signed16(imm)) & 0xFFFFFFFF
                refs.append((a, mn, resolved, 'r13'))
            elif ra in hi:
                base = hi[ra]
                resolved = (base + (imm if op == 24 else signed16(imm))) & 0xFFFFFFFF
                refs.append((a, mn, resolved, 'ha/l'))
            # addi/ori write rd. A resolved address becomes rd's new tracked
            # base, so a later instruction chained off rd (very common for
            # `lis r8,sym@ha; addi r8,r8,sym@l` where rd==ra) computes from
            # the just-resolved address rather than a stale pre-add one.
            if op in (14, 24):
                if resolved is not None:
                    hi[rd] = resolved
                else:
                    hi.pop(rd, None)
            elif op in DFORM and 32 <= op <= 47:
                # integer load writes rd
                if op in (32, 33, 34, 35, 40, 41, 42, 43, 46):
                    hi.pop(rd, None)

            # An update-form load/store writes the effective address back into
            # rA. Track it, or every later reference chained off rA resolves
            # against a stale base -- see the note on UPDATE above.
            if op in UPDATE:
                if resolved is not None:
                    hi[ra] = resolved
                else:
                    hi.pop(ra, None)
        else:
            # crude clobber tracking for other rD-writing forms
            if op in (7, 8, 12, 13, 28, 29):     # mulli, subfic, addic(.), andi./andis.
                hi.pop(rd, None)
            elif op == 31:
                xo = (w >> 1) & 0x3FF
                # most X/XO forms write rD (bits 21-25); stores/compares do not
                if xo not in (0, 32, 4, 150, 183, 151, 215, 407, 439, 191, 247,
                              663, 727, 695, 759, 512, 144, 467, 210, 242, 274):
                    hi.pop(rd, None)
        a += 4
    return refs


def main():
    dol = sys.argv[1]
    start = int(sys.argv[2], 16)
    end = int(sys.argv[3], 16)
    sda = 0x8042F980
    sda2 = 0x80433360
    for i, arg in enumerate(sys.argv):
        if arg == '--sda':
            sda = int(sys.argv[i + 1], 16)
        if arg == '--sda2':
            sda2 = int(sys.argv[i + 1], 16)
    d, secs = load_dol(dol)
    refs = scan(d, secs, start, end, sda, sda2)
    for a, mn, t, kind in refs:
        print('%08x %-6s %08x %s' % (a, mn, t, kind))


if __name__ == '__main__':
    main()
