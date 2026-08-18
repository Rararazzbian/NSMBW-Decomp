with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

# Let's inspect the destructors and constructors specifically
print("=== DESTRUCTORS & CONSTRUCTORS ===")
for block in text.split('=================================================='):
    if any(k in block for k in ['__ct__', '__dt__', 'onExit', 'onEnter', 'run__Q23EGG6ThreadFv', 'fn_800CF170', 'getSaveData', 'setNandError']):
        print(block)
