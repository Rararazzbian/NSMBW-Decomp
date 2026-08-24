#!/usr/bin/env python3
"""Dump retail DOL bytes around a virtual address; print section table."""
import struct
import sys

DOL = '/opt/NSMBW-Decomp/bin/wiimj2d.dol'

with open(DOL, 'rb') as f:
    hdr = f.read(0x100)
    data = f.read()

off = struct.unpack('>18I', hdr[0x00:0x48])
addr = struct.unpack('>18I', hdr[0x48:0x90])
size = struct.unpack('>18I', hdr[0x90:0xD8])

va = int(sys.argv[1], 16)
before = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x10
after = int(sys.argv[3], 16) if len(sys.argv) > 3 else 0x20

for i in range(18):
    if off[i] and addr[i]:
        print(f"sec[{i:2d}] addr=0x{addr[i]:08X} size=0x{size[i]:06X} off=0x{off[i]:06X}")

target = None
for i in range(18):
    if off[i] and addr[i] <= va < addr[i] + size[i]:
        target = i
        break

if target is None:
    print(f"0x{va:08X} not in any loaded section")
    sys.exit(1)

fo = off[target] + (va - addr[target])
lo = fo - before
hi = fo + after
print(f"\nVA 0x{va:08X} -> sec[{target}] file 0x{fo:X}")
print(f"context 0x{addr[target] + (lo - off[target]):08X}..0x{addr[target] + (hi - off[target]):08X}:")
chunk = data[lo:hi]
base_va = addr[target] + (lo - off[target])
for j in range(0, len(chunk), 16):
    row = chunk[j:j + 16]
    hexs = ' '.join(f'{b:02X}' for b in row)
    asc = ''.join(chr(b) if 0x20 <= b < 0x7F else '.' for b in row)
    print(f"{base_va + j:08X}  {hexs:<48s} {asc}")
