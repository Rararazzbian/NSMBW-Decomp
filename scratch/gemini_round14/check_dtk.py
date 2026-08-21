import struct

with open('original/wiimj2d.dol', 'rb') as f:
    dol_bytes = f.read()

text_offsets = struct.unpack('>7I', dol_bytes[0:0x1C])
data_offsets = struct.unpack('>11I', dol_bytes[0x1C:0x48])
text_addrs = struct.unpack('>7I', dol_bytes[0x48:0x64])
data_addrs = struct.unpack('>11I', dol_bytes[0x64:0x90])
text_sizes = struct.unpack('>7I', dol_bytes[0x90:0xAC])
data_sizes = struct.unpack('>11I', dol_bytes[0xAC:0xD8])

def va_to_dol_offset(va):
    for o, a, s in zip(text_offsets, text_addrs, text_sizes):
        if a <= va < a + s:
            return o + (va - a)
    for o, a, s in zip(data_offsets, data_addrs, data_sizes):
        if a <= va < a + s:
            return o + (va - a)
    return None

# Disassemble a few instructions at 0x80076BC0 to 0x80076FD0
# Let us check if dtk can disasm
