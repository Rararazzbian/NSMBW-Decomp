import os
import sys
import json
import subprocess
from collections import defaultdict

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import sibmap

def main():
    print("Loading sibmap corpus...")
    corpus = sibmap.load_corpus()
    print(f"Loaded {len(corpus)} corpus functions.")

    dis_dir = os.path.join(ROOT, 'scratch', 'gemini_round12', 'dis_boss')
    os.makedirs(dis_dir, exist_ok=True)
    dtk = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
    
    split_dir = os.path.join(ROOT, 'bin', 'dtkspl', 'd_en_bossNP', 'obj')
    files = sorted(os.listdir(split_dir))
    
    for f in files:
        if f.endswith('_text.o'):
            src = os.path.join(split_dir, f)
            out_txt = os.path.join(dis_dir, f.replace('.o', '.txt'))
            if not os.path.exists(out_txt) or os.path.getsize(out_txt) == 0:
                subprocess.run([dtk, 'elf', 'disasm', src, out_txt], check=True)

    all_parsed_fns = []
    for f in sorted(os.listdir(dis_dir)):
        if f.endswith('.txt'):
            fns = sibmap.parse(os.path.join(dis_dir, f))
            for fn in fns:
                fn['shape'] = [sibmap.shape(w) for w in fn['words']]
                fn['sig'] = sibmap.opsig(fn['words'])
                all_parsed_fns.append(fn)

    print(f"Total parsed boss target functions: {len(all_parsed_fns)}")

    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_all_tus.json')) as f:
        tus = json.load(f)

    scored_tus = []
    for tu in tus:
        t_start = tu['text_start']
        t_end = tu['text_end']
        tu_fns = [f for f in all_parsed_fns if t_start <= f['addr'] < t_end]
        
        total_words = 0
        total_exact_matched_words = 0
        total_shape_matched_words = 0
        
        for f in tu_fns:
            n = len(f['words'])
            total_words += n
            best_exact = 0.0
            best_shape = 0.0
            
            for c in corpus:
                m = len(c['words'])
                if abs(m - n) > max(6, 0.5 * n):
                    continue
                if sibmap.sigsim(f['sig'], c['sig']) < 0.55:
                    continue
                se = sibmap.sim(f['words'], c['words'])
                ss = sibmap.sim(f['shape'], c['shape'])
                if se > best_exact:
                    best_exact = se
                if ss > best_shape:
                    best_shape = ss
                    
            total_exact_matched_words += best_exact * n
            total_shape_matched_words += best_shape * n
            
        exact_score = (total_exact_matched_words / total_words) if total_words > 0 else 0.0
        shape_score = (total_shape_matched_words / total_words) if total_words > 0 else 0.0
        
        tu['parsed_fn_count'] = len(tu_fns)
        tu['exact_sibling_score'] = round(exact_score * 100, 2)
        tu['shape_sibling_score'] = round(shape_score * 100, 2)
        scored_tus.append(tu)

    print("\n=== Sibling Similarity Scores for All 26 Boss TUs ===")
    for tu in scored_tus:
        p_name = tu['profiles'][0][1] if tu['profiles'] else "No Profile"
        print(f"TU {tu['index']:2d} | 0x{tu['text_start']:05x}-0x{tu['text_end']:05x} | Code: {tu['code_bytes']:5d} B | {tu['fn_count']:3d} fns | Sibling: {tu['exact_sibling_score']:5.2f}% exact / {tu['shape_sibling_score']:5.2f}% shape | {p_name}")

    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_scored_tus.json'), 'w') as f:
        json.dump(scored_tus, f, indent=2)

if __name__ == '__main__':
    main()
