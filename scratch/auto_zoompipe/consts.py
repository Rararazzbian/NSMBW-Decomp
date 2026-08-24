import struct

dol = open('/opt/NSMBW-Decomp/bin/wiimj2d.dol', 'rb').read()
offs = struct.unpack('>18I', dol[0x00:0x48])
addrs = struct.unpack('>18I', dol[0x48:0x90])
sizes = struct.unpack('>18I', dol[0x90:0xD8])
for i in range(18):
    if addrs[i] == 0 or sizes[i] == 0:
        continue
    if addrs[i] <= 0x8042BDD8 < addrs[i] + sizes[i]:
        off = offs[i] + (0x8042BDD8 - addrs[i])
        f4 = struct.unpack('>f', dol[off:off + 4])[0]
        f3 = struct.unpack('>f', dol[off + 4:off + 8])[0]
        f5 = struct.unpack('>d', dol[off + 8:off + 16])[0]
        print('sec %x..%x (hdr slot %d)' % (addrs[i], addrs[i] + sizes[i], i))
        print('f4 @8042BDD8 =', f4)
        print('f3 @8042BDDC =', f3)
        print('f5 @8042BDE0 =', f5, dol[off + 8:off + 16].hex())
