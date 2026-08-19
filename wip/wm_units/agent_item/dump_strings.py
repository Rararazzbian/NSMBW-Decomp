import struct, os
REL_PATH = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'original', 'd_basesNP.rel')
data = open(REL_PATH, 'rb').read()
base_data = 0x1d0c00
base_rodata = 0x1c6600

def dump(lo, hi, base, label):
    print('---', label, hex(lo), '-', hex(hi), '---')
    chunk = data[base+lo:base+hi]
    for i in range(0, len(chunk), 16):
        row = chunk[i:i+16]
        hexs = ' '.join('%02x'%b for b in row)
        asci = ''.join(chr(b) if 32<=b<127 else '.' for b in row)
        print(hex(lo+i), hexs, asci)

dump(0x450e4, 0x45138, base_data, 'SI_ table raw strings')
dump(0x45190, 0x451c0, base_data, 'anim name strings + tail')
dump(0x8988, 0x89e0, base_rodata, 'rodata_8988 floats')

print()
print('--- floats decoded ---')
chunk = data[base_rodata+0x8988: base_rodata+0x89d0]
for i in range(0, len(chunk), 4):
    val = struct.unpack_from('>f', chunk, i)[0]
    print(hex(0x8988+i), val)
