import sys, os, re
sys.path.append('.')
from tools.auto_decomp import harness, pool, poolcheck

TARGET_FILES = [
    'scratch/gemini_round22/auto_03_800A8710_text.txt',
    'scratch/gemini_round22/auto_sinit_text.txt',
    'scratch/gemini_round22/auto_03_800B03D8_text.txt'
]
DRAFT_SRC = 'scratch/gemini_round22/d_enemy_toride_kokoopa.cpp'
DRAFT_OBJ = 'scratch/gemini_round22/d_enemy_toride_kokoopa.o'
DRAFT_DIS = 'scratch/gemini_round22/draft_disasm.txt'
EXTRA_INC = ['scratch/gemini_round22/include']

POOLS = None
POOL_FAILURES = []

def parse_disasm(path):
    """Parse a dtk disassembly file into {norm_name: [(bytes_str, insn_str)]}."""
    fns, cur = {}, None
    if not os.path.exists(path):
        return fns
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'^\s*\.fn\s+([^\s,]+)', line)
        if m:
            raw_name = m.group(1).strip('"')
            cur = harness.norm_name(raw_name)
            fns[cur] = []
            continue
        if re.match(r'^\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns

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

def compile_and_disasm(src=DRAFT_SRC, obj=DRAFT_OBJ, dis=DRAFT_DIS):
    ok, err = harness.compile_draft(src, obj, extra_inc=EXTRA_INC, module='wiimj2d')
    if not ok:
        print('COMPILE ERROR:\n', err)
        return False
    ok_d, err_d = harness.disasm(obj, dis)
    if not ok_d:
        print('DISASM ERROR:\n', err_d)
        return False
    return True

def init_pools():
    global POOLS
    try:
        POOLS = (poolcheck.object_pool(DRAFT_OBJ), poolcheck.pool.load())
    except Exception as e:
        POOLS = None
        print(f'WARNING: pooled constants NOT value-checked ({e})')

def is_matched(draft_fn, target_fn, name='?'):
    if not draft_fn or not target_fn:
        return False
    # Union gate: raw bytes equal OR canonicalised text equal
    raw_bytes_match = ([b for b, _ in draft_fn] == [b for b, _ in target_fn])
    if raw_bytes_match:
        pass
    else:
        d_can = harness.canonicalise([t for _, t in draft_fn])
        t_can = harness.canonicalise([t for _, t in target_fn])
        if d_can != t_can:
            return False
    if POOLS is None:
        return True
    bad = poolcheck.compare_pools(target_fn, draft_fn, *POOLS)
    for i, va, tv, dv in bad:
        POOL_FAILURES.append((name, i, va, tv, dv))
    return not bad

def diff_fn(pattern):
    target_all = {}
    for tf in TARGET_FILES:
        target_all.update(parse_disasm(tf))
    draft_all = parse_disasm(DRAFT_DIS)
    
    target_name = None
    for name in target_all:
        if pattern.lower() in name.lower():
            target_name = name
            break
    if not target_name:
        for name in draft_all:
            if pattern.lower() in name.lower():
                target_name = name
                break
    if not target_name:
        print(f'No function matching {pattern}')
        return
    
    t_fn = target_all.get(target_name)
    d_fn = draft_all.get(target_name)
    if t_fn is None:
        print(f'Target function {target_name} not found in target files.')
        return
    if d_fn is None:
        print(f'Draft function {target_name} is NOT emitted in {DRAFT_DIS} (0 B)')
        return
        
    init_pools()
    global POOL_FAILURES
    POOL_FAILURES = []
    matched_flag = is_matched(d_fn, t_fn, target_name)
    
    print(f'=== DIFF: {target_name} ===')
    print(f'Target insns: {len(t_fn)}, Draft insns: {len(d_fn)}')
    max_l = max(len(t_fn), len(d_fn))
    diff_count = 0
    t_can = harness.canonicalise([t for _, t in t_fn])
    d_can = harness.canonicalise([t for _, t in d_fn])
    for i in range(max_l):
        tb = t_fn[i][0] if i < len(t_fn) else '        '
        db = d_fn[i][0] if i < len(d_fn) else '        '
        tt = t_can[i] if i < len(t_can) else '<none>'
        dt = d_can[i] if i < len(d_can) else '<none>'
        eq = (tb == db) or (tt == dt)
        mark = '==' if eq else '!='
        if not eq:
            diff_count += 1
        print(f'{i:3d} {mark}  T: [{tb}] {tt:40s} | D: [{db}] {dt}')
    print(f'Result: {"MATCH!" if matched_flag else f"{diff_count} diffs"}')
    if POOL_FAILURES:
        print('Pool check failures:')
        for name, i, va, tv, dv in POOL_FAILURES:
            print(f'  insn {i}: target 0x{va:08X} = {tv!r}, draft = {dv!r}')

def eval_all():
    tu_syms = get_tu_syms()
    target_all = {}
    for tf in TARGET_FILES:
        target_all.update(parse_disasm(tf))
    draft_all = parse_disasm(DRAFT_DIS)
    draft_fns_sz = dict(harness.list_functions(DRAFT_DIS, with_size=True))
    
    init_pools()
    global POOL_FAILURES
    POOL_FAILURES = []
    
    matched = []
    unmatched = []
    total_matched_bytes = 0
    total_bytes = sum(s[2] for s in tu_syms)
    
    for name, addr, sz in tu_syms:
        norm = harness.norm_name(name)
        t_fn = target_all.get(norm)
        d_fn = draft_all.get(norm)
        dsz = draft_fns_sz.get(norm, 0)
        
        match = False
        diffs = -1
        if t_fn is not None and d_fn is not None:
            match = is_matched(d_fn, t_fn, norm)
            if not match:
                t_can = harness.canonicalise([t for _, t in t_fn])
                d_can = harness.canonicalise([t for _, t in d_fn])
                diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        elif d_fn is not None and t_fn is None:
            diffs = -2 # missing target
        
        if match:
            matched.append((name, addr, sz, dsz))
            total_matched_bytes += sz
        else:
            unmatched.append((name, addr, sz, dsz, diffs, t_fn is not None, d_fn is not None))
            
    print(f'Matched: {len(matched)} / {len(tu_syms)} ({len(matched)/len(tu_syms)*100:.2f}%)')
    print(f'Matched bytes: {total_matched_bytes} / {total_bytes} ({total_matched_bytes/total_bytes*100:.2f}%)')
    
    if POOL_FAILURES:
        print(f'\n{len(POOL_FAILURES)} WRONG CONSTANT(S):')
        for name, i, va, tv, dv in POOL_FAILURES:
            print(f'  {name}: insn {i} retail 0x{va:08X}={tv!r}, draft={dv!r}')
            
    unmatched_by_size = sorted(unmatched, key=lambda x: x[2], reverse=True)
    print('\nTop 20 Unmatched:')
    for i, (name, addr, sz, dsz, diffs, has_t, has_d) in enumerate(unmatched_by_size[:20]):
        status = '0 B (unwritten)' if dsz == 0 else f'{dsz} B ({diffs} diffs)'
        print(f'{i+1:2d}. {name} (0x{addr:08X}): Target={sz} B, Draft={status}')
        
    return matched, unmatched

if __name__ == '__main__':
    if len(sys.argv) > 1:
        cmd = sys.argv[1]
        if cmd == 'eval':
            compile_and_disasm()
            eval_all()
        elif cmd == 'diff':
            compile_and_disasm()
            diff_fn(sys.argv[2])
        elif cmd == 'compile':
            compile_and_disasm()
