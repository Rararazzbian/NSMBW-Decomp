import struct, sys

DOL = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol"
TEXT_VA = 0x80006780
TEXT_FILEOFF = 0x27C0
TEXT_SIZE = 0x2E7544

with open(DOL, 'rb') as f:
    data = f.read()

def ha_lo(addr):
    lo = addr & 0xFFFF
    hi = (addr >> 16) & 0xFFFF
    if lo & 0x8000:
        hi = (hi + 1) & 0xFFFF
    return hi, lo

def scan_range(lo_va, hi_va, target):
    """scan .text instructions in [lo_va,hi_va) for lis rX,ha(target) followed within
    a small window by an instruction using rX with lo(target) as the 16-bit immediate
    (addi/ori/lwz/stw/lfs/stfs/lfd/stfd/lha/lbz/stb/sth)."""
    ha, lo = ha_lo(target)
    hits = []
    start_off = TEXT_FILEOFF + (lo_va - TEXT_VA)
    end_off = TEXT_FILEOFF + (hi_va - TEXT_VA)
    words = []
    off = start_off
    va = lo_va
    while off < end_off:
        w = struct.unpack_from('>I', data, off)[0]
        words.append((va, w))
        off += 4
        va += 4
    for i, (va, w) in enumerate(words):
        op = w >> 26
        if op == 15:  # lis / addis
            rA = (w >> 16) & 0x1F
            rD = (w >> 21) & 0x1F
            imm = w & 0xFFFF
            if rA == 0 and imm == ha:
                # this is 'lis rD, ha' -- look ahead up to 20 instrs for use of rD with lo
                for j in range(i+1, min(i+2000, len(words))):
                    va2, w2 = words[j]
                    op2 = w2 >> 26
                    rA2 = (w2 >> 16) & 0x1F
                    rD2 = (w2 >> 21) & 0x1F
                    imm2 = w2 & 0xFFFF
                    if imm2 & 0x8000:
                        simm2 = imm2 - 0x10000
                    else:
                        simm2 = imm2
                    lo_s = lo if not (lo & 0x8000) else lo - 0x10000
                    if rA2 == rD and op2 in (14,24,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55):
                        if simm2 == lo_s:
                            hits.append((va, va2, rD))
                            break
    return hits

if __name__ == '__main__':
    lo_va = int(sys.argv[1], 16)
    hi_va = int(sys.argv[2], 16)
    target = int(sys.argv[3], 16)
    hits = scan_range(lo_va, hi_va, target)
    for h in hits:
        print(f"lis at {h[0]:#010x}, use at {h[1]:#010x}, reg r{h[2]}")
    if not hits:
        print("no hits")
