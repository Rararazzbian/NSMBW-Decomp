import struct

dol = open('/opt/NSMBW-Decomp/bin/wiimj2d.dol', 'rb').read()
offs = struct.unpack('>18I', dol[0x00:0x48])
addrs = struct.unpack('>18I', dol[0x48:0x90])
sizes = struct.unpack('>18I', dol[0x90:0xD8])
VA = 0x80006780
for i in range(18):
    if addrs[i] == 0 or sizes[i] == 0:
        continue
    if addrs[i] <= VA < addrs[i] + sizes[i]:
        off = offs[i] + (VA - addrs[i])
        start = off + (0x80064104 - VA)
        chunk = dol[start:start + 0xC]
        print('bytes at 0x80064104:', chunk.hex(), 'all-zero:', chunk == b'\x00' * 12)
