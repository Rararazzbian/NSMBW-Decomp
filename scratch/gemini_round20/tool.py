import sys, os, re
sys.path.append('.')
from tools.auto_decomp import harness, pool, poolcheck

TARGET_FILES = [
    'scratch/gemini_round20/auto_03_800A8710_text.txt',
    'scratch/gemini_round20/auto_sinit_text.txt',
    'scratch/gemini_round20/auto_03_800B03D8_text.txt'
]
DRAFT_SRC = 'scratch/gemini_round20/d_enemy_toride_kokoopa.cpp'
DRAFT_OBJ = 'scratch/gemini_round20/d_enemy_toride_kokoopa.o'
DRAFT_DIS = 'scratch/gemini_round20/draft_disasm.txt'
EXTRA_INC = ['scratch/gemini_round20/include']

def get_tu_syms():
    sym_re = re.compile(r'^(\S+)\s*=\s*\.text:(0x[0-9A-Fa-f]+);\s*//\s*type:function\s*size:(0x[0-9A-Fa-f]+)')
    tu_syms = []
    with open('bin/dtk/wiimj2d_symbols.txt', encoding='utf-8') as f:
        for line in f:
            m = sym_re.match(line.strip())
            if m:
                name, addr_s, sz_s = m.group(1), m.group(2), m.group(3)
                addr = int(addr_s, 16)
                sz = int(sz_s, 16)
                if 0x800A8710 <= addr < 0x800B0A20:
                    tu_syms.append((name, addr, sz))
    return tu_syms

def compile_and_disasm():
    ok, err = harness.compile_draft(DRAFT_SRC, DRAFT_OBJ, extra_inc=EXTRA_INC, module='wiimj2d')
    if not ok:
        print('COMPILE ERROR:\n', err)
        return False
    ok_d, err_d = harness.disasm(DRAFT_OBJ, DRAFT_DIS)
    if not ok_d:
        print('DISASM ERROR:\n', err_d)
        return False
    return True

def find_target_fn(pattern):
    for tf in TARGET_FILES:
        with open(tf, encoding='utf-8') as f:
            lines = f.readlines()
        inside = False
        cur_name = None
        cur_lines = []
        for line in lines:
            m = re.match(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+', line.strip())
            if m:
                norm = harness.norm_name(m.group(1))
                if pattern.lower() in norm.lower():
                    inside = True
                    cur_name = norm
                    cur_lines = [line]
                    continue
            if inside:
                cur_lines.append(line)
                if line.strip().startswith('.endfn'):
                    return cur_name, cur_lines, tf
    return None, None, None

def inspect_target(pattern):
    name, lines, tf = find_target_fn(pattern)
    if not name:
        print(f'No target function matching {pattern}')
        return
    print(f'=== TARGET: {name} ({tf}) ===')
    pool_sym_re = re.compile(r'@\d+_([0-9A-Fa-f]{8})')
    for l in lines:
        l_str = l.rstrip()
        pool_matches = pool_sym_re.findall(l_str)
        note = ''
        if pool_matches:
            for va_str in pool_matches:
                va = int(va_str, 16)
                res = pool.read(va)
                if res:
                    note += f' [POOL 0x{va:08X}: f32={res["f32"]!r} f64={res.get("f64")!r}]'
        print(l_str + note)

def diff_fn(pattern):
    name, lines, tf = find_target_fn(pattern)
    if not name:
        print(f'No target function matching {pattern}')
        return
    
    target_body = harness.extract(tf, name)
    draft_body = harness.extract(DRAFT_DIS, name)
    
    if target_body is None:
        print(f'Failed to extract target body for {name}')
        return
    if draft_body is None:
        print(f'Draft function {name} is NOT emitted in {DRAFT_DIS} (0 B)')
        return
    
    print(f'=== DIFF: {name} ===')
    print(f'Target insns: {len(target_body)}, Draft insns: {len(draft_body)}')
    max_l = max(len(target_body), len(draft_body))
    diff_count = 0
    for i in range(max_l):
        t = target_body[i] if i < len(target_body) else '<none>'
        d = draft_body[i] if i < len(draft_body) else '<none>'
        eq = (t == d)
        if not eq:
            diff_count += 1
            mark = '!='
        else:
            mark = '=='
        print(f'{i:3d} {mark}  T: {t:40s} | D: {d}')
    print(f'Result: {"MATCH!" if diff_count == 0 else f"{diff_count} diffs"}')

def eval_all():
    tu_syms = get_tu_syms()
    draft_fns = dict(harness.list_functions(DRAFT_DIS, with_size=True))
    
    matched = []
    unmatched = []
    total_matched_bytes = 0
    total_bytes = sum(s[2] for s in tu_syms)
    
    for name, addr, sz in tu_syms:
        target_body = None
        for tf in TARGET_FILES:
            b = harness.extract(tf, name)
            if b:
                target_body = b
                break
        draft_body = harness.extract(DRAFT_DIS, name)
        draft_sz = draft_fns.get(name, 0)
        
        is_match = False
        diffs = -1
        if target_body is not None and draft_body is not None:
            if target_body == draft_body or harness.canonicalise(target_body) == harness.canonicalise(draft_body):
                is_match = True
            else:
                diffs = sum(1 for t, d in zip(target_body, draft_body) if t != d) + abs(len(target_body) - len(draft_body))

        if is_match:
            matched.append((name, addr, sz, draft_sz))
            total_matched_bytes += sz
        else:
            unmatched.append((name, addr, sz, draft_sz, diffs, target_body is not None, draft_body is not None))
            
    print(f'Matched: {len(matched)} / {len(tu_syms)} ({len(matched)/len(tu_syms)*100:.2f}%)')
    print(f'Matched bytes: {total_matched_bytes} / {total_bytes} ({total_matched_bytes/total_bytes*100:.2f}%)')
    
    unmatched_by_size = sorted(unmatched, key=lambda x: x[2], reverse=True)
    print('\nTop 20 Unmatched:')
    for i, (name, addr, sz, dsz, diffs, has_t, has_d) in enumerate(unmatched_by_size[:20]):
        status = '0 B (unwritten)' if dsz == 0 else f'{dsz} B ({diffs} diffs)'
        print(f'{i+1:2d}. {name} (0x{addr:08X}): Target={sz} B, Draft={status}')

if __name__ == '__main__':
    if len(sys.argv) > 1:
        cmd = sys.argv[1]
        if cmd == 'eval':
            compile_and_disasm()
            eval_all()
        elif cmd == 'inspect':
            inspect_target(sys.argv[2])
        elif cmd == 'diff':
            compile_and_disasm()
            diff_fn(sys.argv[2])
        elif cmd == 'compile':
            compile_and_disasm()
