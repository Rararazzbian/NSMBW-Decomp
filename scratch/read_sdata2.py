import struct, re

with open('original/wiimj2d.dol', 'rb') as f:
    dol = f.read()

offsets = struct.unpack('>18I', dol[0:72])
addrs = struct.unpack('>18I', dol[72:144])
sizes = struct.unpack('>18I', dol[144:216])

def read_addr(vaddr, length):
    for i in range(18):
        if addrs[i] <= vaddr < addrs[i] + sizes[i]:
            file_off = offsets[i] + (vaddr - addrs[i])
            return dol[file_off : file_off + length]
    return None

print("=== Checking .sdata2: 0x8042B628 .. 0x8042B680 ===")
for a in range(0x8042B628, 0x8042B680, 4):
    b = read_addr(a, 4)
    val_f = struct.unpack('>f', b)[0]
    val_u = struct.unpack('>I', b)[0]
    print(f"  {hex(a)}: float={val_f:10.4f} hex=0x{val_u:08x}")

