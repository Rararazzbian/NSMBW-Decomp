import struct
import re
import os
import json
import subprocess
import difflib
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
DIS = os.path.join(ROOT, 'tools', 'dis')
OBJ_DIR = os.path.join(ROOT, 'bin', 'dtkspl', 'd_enemiesNP', 'obj')
TGT_DIS = os.path.join(ROOT, 'scratch', 'dis_enemies')
os.makedirs(TGT_DIS, exist_ok=True)

def shape(w):
    op = w >> 26
    if op == 18:
        return (op << 26) | (w & 3)
    if op == 16:
        return w & 0xFFFF0003
    if op in (7, 8, 10, 11, 12, 13, 14, 15) or 32 <= op <= 55:
        return w & 0xFFFF0000
    if op in (20, 21, 23):
        return w
    return w

def stats(a, b):
    sm = difflib.SequenceMatcher(a=a, b=b, autojunk=False)
    aligned = sum(bl.size for bl in sm.get_matching_blocks())
    return aligned

FN_RE = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*(\w+)\s*$')
END_RE = re.compile(r'^\.endfn')
INSN_RE = re.compile(r'^/\*\s*([0-9A-F]{8})\s+[0-9A-F]{8}\s+((?:[0-9A-F]{2} ){3}[0-9A-F]{2})\s*\*/\s*(.*)$')

def parse_disasm(path):
    fns = []
    cur = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_RE.match(s)
            if m:
                cur = {'name': m.group(1), 'addr': None, 'words': [], 'texts': []}
                continue
            if END_RE.match(s):
                if cur and cur['words']:
                    fns.append(cur)
                cur = None
                continue
            if cur is None:
                continue
            m = INSN_RE.match(s)
            if m:
                if cur['addr'] is None:
                    cur['addr'] = int(m.group(1), 16)
                w = int(m.group(2).replace(' ', ''), 16)
                cur['words'].append(w)
                cur['texts'].append(m.group(3).split('/*')[0].strip())
    return [f for f in fns if not f['name'].startswith('gap_') and not f['name'].startswith('pad_')]

# 1. Load corpus
corpus = []
for f in sorted(os.listdir(DIS)):
    if f.startswith('corpus_') and f.endswith('.txt'):
        tag = f[len('corpus_'):-len('.txt')]
        for fn in parse_disasm(os.path.join(DIS, f)):
            fn['tu'] = tag
            fn['shape'] = [shape(w) for w in fn['words']]
            corpus.append(fn)

print(f'Loaded {len(corpus)} corpus functions')

# 2. Disassemble all d_enemiesNP split objects
obj_files = sorted([f for f in os.listdir(OBJ_DIR) if f.endswith('.o')])
print(f'Disassembling {len(obj_files)} objects...')
for of in obj_files:
    txt_name = of[:-2] + '.txt'
    txt_path = os.path.join(TGT_DIS, txt_name)
    if not os.path.exists(txt_path):
        subprocess.run([DTK, 'elf', 'disasm', os.path.join(OBJ_DIR, of), txt_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# 3. Load all target functions from disassembled files
tgt_fns_all = []
for f in sorted(os.listdir(TGT_DIS)):
    if f.endswith('.txt'):
        tgt_fns_all.extend(parse_disasm(os.path.join(TGT_DIS, f)))

tgt_fns_all.sort(key=lambda f: f['addr'])
print(f'Total target functions parsed: {len(tgt_fns_all)}')

# 4. Load TUs from JSON
with open(os.path.join(ROOT, 'scratch', 'd_enemiesNP_tus.json')) as f:
    tus = json.load(f)

# Helper to score a TU
def score_tu(tu):
    t_start = int(tu['start'], 16)
    t_end = int(tu['end'], 16)
    fns = [f for f in tgt_fns_all if t_start <= f['addr'] < t_end]
    if not fns:
        return 0.0, 0.0, 0, 0
    total_words = sum(len(f['words']) for f in fns)
    if total_words == 0:
        return 0.0, 0.0, 0, 0
    exact_aligned = 0
    shape_aligned = 0
    for f in fns:
        n = len(f['words'])
        f_shape = [shape(w) for w in f['words']]
        best_e = 0
        best_s = 0
        for c in corpus:
            m = len(c['words'])
            if abs(m - n) > max(6, 0.5 * n):
                continue
            al_e = stats(f['words'], c['words'])
            al_s = stats(f_shape, c['shape'])
            if al_e > best_e:
                best_e = al_e
            if al_s > best_s:
                best_s = al_s
        exact_aligned += best_e
        shape_aligned += best_s
    return exact_aligned / total_words, shape_aligned / total_words, len(fns), total_words * 4

# Score candidate TUs
scored_tus = []
for tu in tus:
    if tu['code'] < 8000 and tu['fns'] >= 4:
        se, ss, fn_cnt, b_cnt = score_tu(tu)
        scored_tus.append((se, ss, tu, fn_cnt, b_cnt))
        print(f"TU {tu['tu_idx']:2d} ({tu['profs']}): code={b_cnt}B, exact={se*100:.2f}%, shape={ss*100:.2f}%")

scored_tus.sort(key=lambda x: -(x[0] * 2 + x[1]))

with open(os.path.join(ROOT, 'scratch', 'scored_enemies_tus.json'), 'w') as f:
    json.dump([{
        'tu_idx': x[2]['tu_idx'],
        'profs': x[2]['profs'],
        'start': x[2]['start'],
        'end': x[2]['end'],
        'span': x[2]['span'],
        'code': x[4],
        'fns': x[3],
        'exact': round(x[0], 4),
        'shape': round(x[1], 4)
    } for x in scored_tus], f, indent=2)

print('Done scoring.')
