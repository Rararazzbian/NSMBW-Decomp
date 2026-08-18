import re

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    text = f.read()

# Let's split by .fn
sections = text.split('.fn ')
header = sections[0]
functions = []

for s in sections[1:]:
    lines = s.splitlines()
    fn_header = lines[0] # e.g. "__ct__13dNandThread_cFiPQ23EGG4Heap, global"
    fn_name = fn_header.split(',')[0].strip()
    functions.append((fn_name, lines))

print(f"Total functions parsed: {len(functions)}")

# Let's write a detailed function analyzer
with open('scratch/gemini_round3/detailed_analysis.txt', 'w') as out:
    for i, (name, lns) in enumerate(functions):
        out.write(f"\n==================================================\n")
        out.write(f"Function {i+1}: {name}\n")
        out.write(f"==================================================\n")
        for line in lns:
            out.write(line + "\n")

print("Saved detailed_analysis.txt")
