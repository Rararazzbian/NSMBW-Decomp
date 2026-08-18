import os
import json
import difflib
import re
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
DIS = os.path.join(ROOT, 'tools', 'dis')
TGT_DIS = os.path.join(ROOT, 'scratch', 'dis_enemies')

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

def opsig(words):
    h = defaultdict(int)
    for w in words:
        h[w >> 26] += 1
    return h

def sigsim(h1, h2):
    keys = set(h1) | set(h2)
    inter = sum(min(h1.get(k, 0), h2.get(k, 0)) for k in keys)
    tot = max(sum(h1.values()), sum(h2.values()), 1)
    return inter / tot

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

print('Loading corpus...')
corpus = []
for f in sorted(os.listdir(DIS)):
    if f.startswith('corpus_') and f.endswith('.txt'):
        tag = f[len('corpus_'):-len('.txt')]
        for fn in parse_disasm(os.path.join(DIS, f)):
            fn['tu'] = tag
            fn['shape'] = [shape(w) for w in fn['words']]
            fn['sig'] = opsig(fn['words'])
            corpus.append(fn)

tgt_fns_all = []
for f in sorted(os.listdir(TGT_DIS)):
    if f.endswith('.txt'):
        tgt_fns_all.extend(parse_disasm(os.path.join(TGT_DIS, f)))

tgt_fns_all.sort(key=lambda f: f['addr'])

with open(os.path.join(ROOT, 'scratch', 'd_enemiesNP_tus.json')) as f:
    tus = json.load(f)

# Candidate TU indices to score in priority:
# 81 (jimen_pakkun), 17 (block_cloud), 73 (icebros), 148 (togezo), 107 (net_nokonoko_lr),
# 48 (left_dokan_pakkun), 37 (coin_jump), 19 (block_soroban), 140 (super_bigpile_left), 151 (waki_jugem)
cand_indices = [81, 17, 73, 148, 107, 48, 37, 19, 140, 151, 21, 155, 44, 38, 49, 50, 108]

results = []
for idx in cand_indices:
    tu = tus[idx]
    t_start = int(tu['start'], 16)
    t_end = int(tu['end'], 16)
    fns = [f for f in tgt_fns_all if t_start <= f['addr'] < t_end]
    total_words = sum(len(f['words']) for f in fns)
    exact_aligned = 0
    shape_aligned = 0
    for f in fns:
        n = len(f['words'])
        f_shape = [shape(w) for w in f['words']]
        f_sig = opsig(f['words'])
        best_e = 0
        best_s = 0
        for c in corpus:
            m = len(c['words'])
            if abs(m - n) > max(6, 0.5 * n):
                continue
            if sigsim(f_sig, c['sig']) < 0.55:
                continue
            al_e = stats(f['words'], c['words'])
            al_s = stats(f_shape, c['shape'])
            if al_e > best_e:
                best_e = al_e
            if al_s > best_s:
                best_s = al_s
        exact_aligned += best_e
        shape_aligned += best_s
    e_pct = (exact_aligned / total_words) * 100 if total_words else 0
    s_pct = (shape_aligned / total_words) * 100 if total_words else 0
    results.append({
        'tu_idx': idx,
        'profs': tu['profs'],
        'start': tu['start'],
        'end': tu['end'],
        'code': total_words * 4,
        'span': tu['span'],
        'fns': len(fns),
        'exact': round(e_pct, 2),
        'shape': round(s_pct, 2)
    })
    p_names = ', '.join(tu['profs'])
    print(f"TU {idx:3d} ({p_names}): {total_words*4} B code ({len(fns)} fns) | Exact: {e_pct:.2f}%, Shape: {s_pct:.2f}%")

with open(os.path.join(ROOT, 'scratch', 'priority_enemies_scored.json'), 'w') as f:
    json.dump(results, f, indent=2)
