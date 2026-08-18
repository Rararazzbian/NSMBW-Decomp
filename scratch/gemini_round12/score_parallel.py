import os
import sys
import json
from concurrent.futures import ProcessPoolExecutor

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import sibmap

def score_fn(args):
    f, corpus_subset = args
    n = len(f['words'])
    f_sig = f['sig']
    f_words = f['words']
    f_shape = f['shape']
    
    best_exact = 0.0
    best_shape = 0.0
    
    for c_words, c_shape, c_sig, c_len in corpus_subset:
        if abs(c_len - n) > max(6, 0.5 * n):
            continue
        if sibmap.sigsim(f_sig, c_sig) < 0.55:
            continue
        se = sibmap.sim(f_words, c_words)
        ss = sibmap.sim(f_shape, c_shape)
        if se > best_exact:
            best_exact = se
        if ss > best_shape:
            best_shape = ss
            if best_exact == 1.0 and best_shape == 1.0:
                break
    return (f['addr'], n, best_exact, best_shape)

def main():
    print("Loading corpus...")
    corpus = sibmap.load_corpus()
    corpus_data = [(c['words'], c['shape'], c['sig'], len(c['words'])) for c in corpus]
    print(f"Loaded {len(corpus_data)} corpus functions.")

    dis_dir = os.path.join(ROOT, 'scratch', 'gemini_round12', 'dis_boss')
    all_parsed_fns = []
    for f in sorted(os.listdir(dis_dir)):
        if f.endswith('.txt'):
            fns = sibmap.parse(os.path.join(dis_dir, f))
            for fn in fns:
                fn['shape'] = [sibmap.shape(w) for w in fn['words']]
                fn['sig'] = sibmap.opsig(fn['words'])
                all_parsed_fns.append(fn)

    print(f"Scoring {len(all_parsed_fns)} functions in parallel...")
    tasks = [(fn, corpus_data) for fn in all_parsed_fns]
    
    fn_scores = {}
    with ProcessPoolExecutor() as executor:
        results = executor.map(score_fn, tasks, chunksize=50)
        for addr, n, be, bs in results:
            fn_scores[addr] = (n, be, bs)

    print("Scoring complete! Aggregating by TU...")
    with open(os.path.join(ROOT, 'scratch', 'gemini_round12', 'boss_all_tus.json')) as f:
        tus = json.load(f)

    scored_tus = []
    for tu in tus:
        t_start = tu['text_start']
        t_end = tu['text_end']
        tu_fns = [f for f in all_parsed_fns if t_start <= f['addr'] < t_end]
        
        total_words = 0
        total_exact_words = 0
        total_shape_words = 0
        
        for f in tu_fns:
            if f['addr'] in fn_scores:
                n, be, bs = fn_scores[f['addr']]
                total_words += n
                total_exact_words += be * n
                total_shape_words += bs * n

        exact_score = (total_exact_words / total_words) if total_words > 0 else 0.0
        shape_score = (total_shape_words / total_words) if total_words > 0 else 0.0
        
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
