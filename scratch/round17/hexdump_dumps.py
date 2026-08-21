import os

base_dir = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\scratch\round17'

def hexdump(path, base):
    raw = open(path, 'rb').read()
    for i in range(0, len(raw), 16):
        chunk = raw[i:i+16]
        hexs = ' '.join('%02X' % b for b in chunk)
        asc = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
        print('%04X  %-47s  %s' % (base + i, hexs, asc))
    print()

print('=== .data 0x8030F820 l_object_name + l_Pa3_rail + ... (0x248) ===')
hexdump(os.path.join(base_dir, 'data_8030F820.bin'), 0x8030F820)
print('=== .data 0x8030FFF4 strings + vtable (0x70) ===')
hexdump(os.path.join(base_dir, 'data_8030FFF4.bin'), 0x8030FFF4)
print('=== .rodata 0x802EFC68 (0x30) ===')
hexdump(os.path.join(base_dir, 'rodata_802EFC68.bin'), 0x802EFC68)
print('=== .sdata2 0x8042C130 (0x50) ===')
hexdump(os.path.join(base_dir, 'sdata2_8042C130.bin'), 0x8042C130)
