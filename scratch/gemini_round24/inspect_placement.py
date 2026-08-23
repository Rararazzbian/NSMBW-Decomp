import sys, os, struct
module_dir = 'scratch/gemini_round24'
sys.path.append('.')
from scratch.gemini_round24 import tool
from scratch.gemini_round24.inspect_data import parse_elf

def check_obj(obj_path='scratch/gemini_round24/d_enemy_toride_kokoopa.o'):
    sections, symbols = parse_elf(obj_path)
    data_sec = [s for s in sections if s['name'] == '.data'][0]
    print(f"=== .data Section: Size = 0{data_sec['size']:04X} ===")
    for sym in sorted(symbols, key=lambda x: x['value']):
        if sym['shndx'] == data_sec['index']:
            print(f"0x{sym['value']:04X} (+0{sym['size']:04X}) {sym['name']}")


if __name__ == '__main__':
    ok = tool.compile_and_disasm()
    if ok:
        check_obj()
