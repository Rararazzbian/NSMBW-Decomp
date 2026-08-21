"""Verify the round-20 d_bg_actor_mng.cpp claims: byte-authoritative tally,
reporting for each function whether match is by RAW BYTES, CANONICALISED TEXT,
or both -- and print the raw byte columns too for pool-constant follow-up.

Usage:  python tally_verify.py
"""
import os, re, sys
sys.path.insert(0, r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp\tools\auto_decomp')
import harness

HERE = os.path.dirname(os.path.abspath(__file__))
TARGET = os.path.join(HERE, 'target.txt')
SRC = os.path.join(HERE, 'draft.cpp')
SHADOW = os.path.join(HERE, 'shadow')


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


def main():
    obj = os.path.join(HERE, 'draft.o')
    txt = os.path.join(HERE, 'draft.txt')
    ok, err = harness.compile_draft(SRC, obj, extra_inc=[SHADOW], module='wiimj2d')
    if not ok:
        print('COMPILE FAILED\n' + err)
        return 1
    dok, derr = harness.disasm(obj, txt)
    if not dok:
        print('DISASM FAILED\n' + derr)
        return 1

    d = parse(txt)
    t = parse(TARGET)

    order = harness.list_functions(TARGET)  # target order, matches QWEN table order
    print(f'{"Function":<55} {"Tgt":>4} {"Drf":>4}  Gate')
    print('-' * 90)
    n_match_bytes = n_match_canon = n_match_either = 0
    for fn in order:
        tfn = t.get(fn)
        dfn = d.get(fn)
        if tfn is None:
            print(f'{fn:<55}  TARGET MISSING FROM PARSE')
            continue
        if dfn is None:
            print(f'{fn:<55} {len(tfn):>4}    -  DRAFT MISSING (no definition emitted)')
            continue
        byte_eq = [b for b, _ in dfn] == [b for b, _ in tfn]
        canon_eq = (harness.canonicalise([x for _, x in dfn]) ==
                    harness.canonicalise([x for _, x in tfn]))
        if byte_eq:
            n_match_bytes += 1
        if canon_eq:
            n_match_canon += 1
        gate = []
        if byte_eq:
            gate.append('BYTES')
        if canon_eq:
            gate.append('CANON')
        if byte_eq or canon_eq:
            n_match_either += 1
            gate_s = '+'.join(gate) + '  MATCH'
        else:
            gate_s = 'DIFFER'
        print(f'{fn:<55} {len(tfn):>4} {len(dfn):>4}  {gate_s}')

    print('-' * 90)
    print(f'TOTAL matched (union gate): {n_match_either} / {len(order)}')
    print(f'  of which byte-exact      : {n_match_bytes}')
    print(f'  of which canon-only      : {n_match_either - n_match_bytes}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
