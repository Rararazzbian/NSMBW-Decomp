import struct

with open('original/wiimj2d.dol', 'rb') as f:
    dol = f.read()

text_offsets = struct.unpack('>7I', dol[0x00:0x1C])
data_offsets = struct.unpack('>11I', dol[0x1C:0x48])
text_addrs = struct.unpack('>7I', dol[0x48:0x64])
data_addrs = struct.unpack('>11I', dol[0x64:0x90])
text_sizes = struct.unpack('>7I', dol[0x90:0xAC])
data_sizes = struct.unpack('>11I', dol[0xAC:0xD8])

def read_dol_bytes(addr, size):
    for i in range(7):
        if text_addrs[i] <= addr < text_addrs[i] + text_sizes[i]:
            off = text_offsets[i] + (addr - text_addrs[i])
            return dol[off:off+size]
    for i in range(11):
        if data_addrs[i] <= addr < data_addrs[i] + data_sizes[i]:
            off = data_offsets[i] + (addr - data_addrs[i])
            return dol[off:off+size]
    return None

for addr in range(0x802F0C00, 0x802F0C80, 0x20):
    b = read_dol_bytes(addr, 0x20)
    print(f"0x{addr:08X}: {b.hex(' ')}")
