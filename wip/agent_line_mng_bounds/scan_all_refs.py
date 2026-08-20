import struct, sys

DOL = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp\original\wiimj2d.dol"
TEXT_VA = 0x80006780
TEXT_FILEOFF = 0x27C0

with open(DOL, 'rb') as f:
    data = f.read()

USE_OPS = set([14,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55])

def scan(text_lo, text_hi, data_lo, data_hi):
    start_off = TEXT_FILEOFF + (text_lo - TEXT_VA)
    end_off = TEXT_FILEOFF + (text_hi - TEXT_VA)
    words = []
    va = text_lo
    off = start_off
    while off < end_off:
        w = struct.unpack_from('>I', data, off)[0]
        words.append((va, w))
        off += 4
        va += 4
    results = []
    for i, (va, w) in enumerate(words):
        op = (w >> 26) & 0x3F
        if op == 15:
            rA = (w >> 16) & 0x1F
            rD = (w >> 21) & 0x1F
            hi = w & 0xFFFF
            if rA != 0:
                continue  # only pure 'lis' (upper-address-from-scratch), skip addis-on-reg
            for j in range(i+1, min(i+400, len(words))):
                va2, w2 = words[j]
                op2 = (w2 >> 26) & 0x3F
                rA2 = (w2 >> 16) & 0x1F
                rD2 = (w2 >> 21) & 0x1F
                imm2 = w2 & 0xFFFF
                if rA2 == rD and op2 in USE_OPS:
                    simm2 = imm2 - 0x10000 if (imm2 & 0x8000) else imm2
                    addr = ((hi << 16) + simm2) & 0xFFFFFFFF
                    if data_lo <= addr < data_hi:
                        results.append((va, va2, addr, rD2))
                    # keep scanning further uses of same rD too (don't break) but avoid infinite;
                    # only take first use per lis to limit noise
                    break
    return results

if __name__ == '__main__':
    text_lo = int(sys.argv[1], 16)
    text_hi = int(sys.argv[2], 16)
    data_lo = int(sys.argv[3], 16)
    data_hi = int(sys.argv[4], 16)
    for r in scan(text_lo, text_hi, data_lo, data_hi):
        print(f"lis@{r[0]:#010x} use@{r[1]:#010x} -> {r[2]:#010x} (r{r[3]})")
