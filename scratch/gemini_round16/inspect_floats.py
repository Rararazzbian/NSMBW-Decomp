import re, struct

with open('scratch/gemini_round16/auto_03_800A8710_text.txt', 'r', encoding='utf-8') as f:
    text = f.read()

with open('original/wiimj2d.dol', 'rb') as f:
    dol = f.read()

raw_fns = re.findall(r'\.fn\s+([^,]+),\s*(.*?)\n([^\n]*)?(?=\.fn|\Z)', text, re.DOTALL)
want_names = {
    'getJumpDist__18dEnTorideKokoopa_cCFv',
    'getKokoopaOffFrm__18dEnTorideKokoopa_cCFv',
    'getShellOnFrm__18dEnTorideKokoopa_cCFv',
    'getKokoopaOnFrm__18dEnTorideKokoopa_cCFv',
    'getShellOffFrm__18dEnTorideKokoopa_cCFv',
    'getCreateBlitzFrm__18dEnTorideKokoopa_cCFv',
    'getShootFrm__18dEnTorideKokoopa_cCFv',
    'getShellChangeEffectOffsetY__18dEnTorideKokoopa_cCFv',
    'getJumpGravity__18dEnTorideKokoopa_cFv',
    'getMagicStickEffectOffset__18dEnTorideKokoopa_cCFv'÷

for fn_name, linkage, body in raw_fns:
    name_clean = fn_name.strip('"')
    if name_clean in want_names:
        print(f'=== {name_clean} ===')
        for l in body.splitlines():
            if '/:' in l:
                print('  ', l.strip())
                m = re.search(r'/\*\s*([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f\s]{11})\s+\*/\s(.*)', l)
                if m:
                    asm = m.group(3)
                    if 'lfs' in asm:
                        m2 = re.search(r'/\*\*\s*((sda\0(\d+\)|\\S++\(r\d+))', asm)
                        print('      -> asm:', asm)
