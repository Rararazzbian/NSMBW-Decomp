import sys, os, re
sys.path.append('.')
from tools.auto_decomp import pool, harness
from scratch.gemini_round22.tool import parse_disasm, TARGET_FILES, DRAFT_DIS

target_all = {}
for tf in TARGET_FILES:
    target_all.update(parse_disasm(tf))
draft_all = parse_disasm(DRAFT_DIS)

def inspect(fn_sub):
    found = False
    for name, t_fn in target_all.items():
        if fn_sub.lower() in name.lower():
            found = True
            print(f"=== TARGET: {name} (insns: {len(t_fn)}) ===")
            for i, (b, t) in enumerate(t_fn):
                note = ""
                m = re.findall(r'@\d+_([0-9A-Fa-f]{8})', t)
                for va_str in m:
                    va = int(va_str, 16)
                    res = pool.read(va)
                    if res:
                        note += f" [0x{va:08X}: f32={res['f32']!r} f64={res.get('f64')!r}]"
                print(f"{i:2d}: [{b}] {t}{note}")
    if not found:
        print(f"Function {fn_sub} not found in target files.")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        inspect(sys.argv[1])
