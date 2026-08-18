with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

for n_fn in ['NANDSimpleSafeOpen', 'NANDSimpleSafeClose', 'NANDSimpleSafeCancel']:
    print(f"\n==================== CALL SITES OF {n_fn} ====================")
    for block in text.split('=================================================='):
        if n_fn in block:
            lines = block.splitlines()
            for i, l in enumerate(lines):
                if n_fn in l:
                    start_i = max(0, i - 10)
                    end_i = min(len(lines), i + 6)
                    print(f"--- in {lines[1] if len(lines)>1 else 'unknown'} ---")
                    for j in range(start_i, end_i):
                        print(lines[j])
