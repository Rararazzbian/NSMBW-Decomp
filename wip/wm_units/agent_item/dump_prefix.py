import os
REL_PATH = os.path.join(os.path.dirname(__file__), '..', '..', '..', 'original', 'd_basesNP.rel')
data = open(REL_PATH, 'rb').read()
base_data = 0x1d0c00

def dump(lo, hi, label):
    print('---', label, hex(lo), '-', hex(hi), '---')
    chunk = data[base_data+lo:base_data+hi]
    for i in range(0, len(chunk), 16):
        row = chunk[i:i+16]
        hexs = ' '.join('%02x'%b for b in row)
        asci = ''.join(chr(b) if 32<=b<127 else '.' for b in row)
        print(hex(lo+i), hexs, asci)

dump(0x44f80, 0x45030, 'preceding .data before g_profile_WM_ITEM')
