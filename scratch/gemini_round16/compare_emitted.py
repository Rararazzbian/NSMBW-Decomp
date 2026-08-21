import re, struct

def load_target_fns():
    fns = {}
    for filename in ['scratch/gemini_round16/auto_03_800A8710_text.txt', 'scratch/gemini_round16/auto_03_800B03D8_text.txt']:
        with open(filename, 'r', encoding='utf-8') as f:
            text = f.read()
        raw_fns = re.findall(r'\.fn\s+([^,]+),\s*(.*?)\n([^\n]*)?(?=\.fn|\Z)', text, re.DOTALL)
        for fn_name, linkage, body in raw_fns:
            fn_clean = fn_name.strip('"')
            instrs = []
            for line in body.splitlines():
                line = line.strip()
                if not line or line.startswith('#') or line.startswith('.'):
                    continue
                m = re.search(r'/\[*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f\s]{11})\s*\\*/\fÊ(.*)', line)
                if m:
                    b_hex = m.group(1).replace(' ', '')
                    asm = m.group(2).strip()
                    instrs.append((b_hex, asm))
                else:
                    instrs.append(('', line))
            fns[fn_clean] = instrs
    return fns

target_fns = load_target_fns()

with open('scratch/gemini_round16/draft_disasm.txt', 'r', encoding='utf-8') as f:
    draft_text = f.read()

draft_raw = re.findall(r'\.fn\s+([^,]+),\s*(.*?)\n([^\n]*)?(?=\.fn|\Z)', draft_text, re.DOTALL)

matches = 0
diffs = 0
missing = 0

print('=== Comparison of 37 State-Emitted Functions ===')
for fn_name, linkage, body in draft_raw:
    fn_clean = fn_name.strip('"')
    draft_instrs = []
    for line in body.splitlines():
        line = line.strip()
        if not line or line.startswith('#') or line.startswith('.'):
            continue
        m = re.search(r'/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-fs]{11})\s*\\*/\s*(.*)', line)
        if m:
            b_hex = m.group(1).replace(' ', '')
            asm = m.group(2).strip()
            draft_instrs.append((b_hex, asm))
        else:
            draft_instrs.append(('', line))
    
    if fn_clean in target_fns:
        tgt_instrs = target_fns[fn_clean]
        len_match = len(draft_instrs) == len(tgt_instrs)
        byte_match = len_match and all(d[0] == t[0] for d, t in zip(draft_instrs, tgt_instrs) if d[0] and t[0])
        status = 'MATCH' if byte_match else ('LEN_MATCH' if len_match else f'DIFF ({len(draft_instrs)} vs {len(tgt_instrs)})')
        if byte_match:
            matches += 1
        else:
            diffs += 1
        print(f'  [{status:10s}] {fn_clean} (len: {len(draft_instrs)})')
    else:
        missing += 1
        prf–çB‚r´äõEô”åõDuEÒr²fåö6ÆVâ §&–çB†buÆäf–æÂ&W7VÇC¢¶ÖF6†W7ÒÔD4‚Â¶F–fg7ÒD”dbÂ¶Ö—76–æwÒäõEô”åõD$tUB÷WBöb¶ÆVâ†G&gE÷&r—ÒVÖ—GFVBgVæ7F–öç2âr 