import os

with open('bin/dtk/wiimj2d_symbols.txt', 'r', encoding='utf-8') as f:
    symbols = f.readlines()

def check_range(sec_name, start_addr, end_addr):
    print(f"=== {sec_name} [0x{start_addr:08X} .. 0x{end_addr:08X}] ===")
    for line in symbols:
        if sec_name in line:
            parts = line.split()
            sec_addr = parts[2].rstrip(';')
            sec, addr_str = sec_addr.split(':')
            addr = int(addr_str, 16)
            if start_addr <= addr <= end_addr:
                print(line.strip())

check_range('.data', 0x80329E20, 0x80329FA0)
check_range('.sdata2', 0x8042DFF0, 0x8042E040)
check_range('.bss', 0x80377F40, 0x803780E0)
check_range('.sbss', 0x8042A720, 0x8042A770)
check_range('.ctors', 0x802EDEF0, 0x802EDF10)
