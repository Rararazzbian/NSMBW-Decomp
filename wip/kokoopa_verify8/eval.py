import sys, re, os
sys.path.append('.')
from tools.auto_decomp import harness, pool

TARGET_FILES = [
    'wip/kokoopa_verify8/auto_03_800A8710_text.txt',
    'wip/kokoopa_verify8/auto_sinit_text.txt',
    'wip/kokoopa_verify8/auto_03_800B03D8_text.txt',
]
DRAFT_SRC = sys.argv[1] if len(sys.argv) > 1 else 'wip/kokoopa_verify8/d_enemy_toride_kokoopa.cpp'
PREFIX = sys.argv[3] if len(sys.argv) > 3 else 'wip/kokoopa_verify8'
DRAFT_OBJ = f'{PREFIX}/draft.o'
DRAFT_DIS = f'{PREFIX}/draft_disasm.txt'
EXTRA_INC = [sys.argv[2]] if len(sys.argv) > 2 else ['wip/kokoopa_verify8/include']

HI = 0x800B0A20

def get_tu_syms():
    out = []
    with open('wip/kokoopa_verify8/target_list.txt', encoding='utf-8') as f:
        for line in f:
            name, addr_s, sz_s = line.strip().split('\t')
            out.append((name, int(addr_s, 16), int(sz_s, 16)))
    return out

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

def extract_from_targets(name):
    for tf in TARGET_FILES:
        b = harness.extract(tf, name)
        if b:
            return b
    return None

def eval_all():
    tu_syms = get_tu_syms()
    draft_fns = dict(harness.list_functions(DRAFT_DIS, with_size=True))

    matched = []
    unmatched = []
    total_matched_bytes = 0
    total_bytes = sum(s[2] for s in tu_syms)

    for name, addr, sz in tu_syms:
        target_body = extract_from_targets(name)
        draft_body = harness.extract(DRAFT_DIS, name)
        draft_sz = draft_fns.get(name, 0)

        # union gate: raw-byte equality OR canonicalised-text equality
        is_match = False
        if target_body is not None and draft_body is not None:
            if target_body == draft_body:
                is_match = True
            else:
                try:
                    ct = harness.canonicalise(target_body)
                    cd = harness.canonicalise(draft_body)
                    if ct == cd:
                        is_match = True
                except Exception:
                    pass
        if is_match:
            matched.append((name, addr, sz, draft_sz))
            total_matched_bytes += sz
        else:
            diffs = -1
            if target_body and draft_body:
                diffs = sum(1 for t, d in zip(target_body, draft_body) if t != d) + abs(len(target_body) - len(draft_body))
            unmatched.append((name, addr, sz, draft_sz, diffs, target_body is not None, draft_body is not None))

    print(f'Matched: {len(matched)} / {len(tu_syms)} ({len(matched)/len(tu_syms)*100:.2f}%)')
    print(f'Matched bytes: {total_matched_bytes} / {total_bytes} ({total_matched_bytes/total_bytes*100:.2f}%)')

    with open(f'{PREFIX}/matched_list.txt', 'w', encoding='utf-8') as f:
        for name, addr, sz, dsz in matched:
            f.write(f'{name}\t0x{addr:08X}\t{sz}\n')

    with open(f'{PREFIX}/unmatched_list.txt', 'w', encoding='utf-8') as f:
        for name, addr, sz, dsz, diffs, has_t, has_d in sorted(unmatched, key=lambda x: x[2], reverse=True):
            status = 'NO-TARGET-BODY' if not has_t else ('0-B-UNWRITTEN' if dsz == 0 else f'{dsz}B_{diffs}diffs')
            f.write(f'{name}\t0x{addr:08X}\t{sz}\t{status}\n')

    return matched, unmatched, total_bytes, total_matched_bytes

if __name__ == '__main__':
    if compile_and_disasm():
        eval_all()
