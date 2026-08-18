with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

for block in text.split('=================================================='):
    if any(k in block for k in ['__dt__Q23EGG5MutexFv', '__dt__6mMutexFv', '__dt__13dNandThread_cFv']):
        print(block)
