import struct

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

for a in [0x80303018, 0x80303068, 0x80427B50, 0x8042B638]:
    data = read_addr(a, 32)
    print(f"Addr {hex(a)}: {data}")
    if data and len(data) >= 4:
        val_f = struct.unpack('>f', data[:4])[0]
        print(f"  as float: {val_f}")

