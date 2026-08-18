with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

for block in text.split('=================================================='):
    if 'existCheck__13dNandThread_cFv' in block:
        print(block)
