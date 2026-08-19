import struct
data = open('original/d_basesNP.rel','rb').read()
base_data = 0x1d0c00
base_rodata = 0x1c6600

def dumphex(lo, hi, base, label):
    print('---', label, hex(lo), '-', hex(hi), '---')
    chunk = data[base+lo:base+hi]
    for i in range(0, len(chunk), 16):
        row = chunk[i:i+16]
        hexs = ' '.join('%02x'%b for b in row)
        asci = ''.join(chr(b) if 32<=b<127 else '.' for b in row)
        print(hex(lo+i), hexs, asci)

dumphex(0x45c90, 0x45d68+0x78, base_data, '.data region')
print()
print('--- rodata_8B10 table floats ---')
chunk = data[base_rodata+0x8b10: base_rodata+0x8b60]
for i in range(0, len(chunk), 4):
    val = struct.unpack_from('>f', chunk, i)[0]
    print(hex(0x8b10+i), val)
