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

check_range('.rodata', 0x802F1430, 0x802F1490)
check_range('.data', 0x80317CA0, 0x80317D20)
check_range('.sbss', 0x8042A270, 0x8042A2B0)
check_range('.sdata', 0x80428000, 0x80429500)
check_range('.sdata2', 0x8042B000, 0x8042E000)
check_range('.bss', 0x80351980, 0x80380000)
