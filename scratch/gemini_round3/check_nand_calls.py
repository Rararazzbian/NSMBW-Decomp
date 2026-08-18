with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

# Search for calls to NAND functions and print the surrounding instructions
nand_fns = [
    'NANDSimpleSafeOpen', 'NANDSimpleSafeClose', 'NANDSimpleSafeCancel',
    'NANDCheck', 'NANDGetType', 'NANDCreate', 'NANDDelete', 'NANDOpen', 'NANDClose',
    'NANDRead', 'NANDWrite', 'NANDGetLength', 'NANDGetHomeDir', 'NANDInitBanner', 'NANDMove',
    'calcCRC32__4sCrcFPCvUl', 'setCurrentHeap__5mHeapFPQ23EGG4Heap'
]

for n_fn in nand_fns:
    print(f"\n==================== CALL SITES OF {n_fn} ====================")
    for block in text.split('=================================================='):
        if n_fn in block:
            lines = block.splitlines()
            for i, l in enumerate(lines):
                if n_fn in l:
                    start_i = max(0, i - 8)
                    end_i = min(len(lines), i + 4)
                    print(f"--- in {lines[1] if len(lines)>1 else 'unknown'} ---")
                    for j in range(start_i, end_i):
                        print(lines[j])
