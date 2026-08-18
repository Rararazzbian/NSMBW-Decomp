with open('scratch/gemini_round3/detailed_analysis.txt', 'r') as f:
    text = f.read()

for block in text.split('=================================================='):
    if any(k in block for k in ['cmdExistCheck', 'cmdSpaceCheck', 'cmdLoad', 'cmdDeleteFile']):
        print(block)
