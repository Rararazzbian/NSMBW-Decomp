with open('scratch/gemini_round8/text_800CE7F0.txt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

in_multi = False
multi_lines = []
for line in lines:
    if '800CE8F0' in line:
        in_multi = True
    if '800CED00' in line:
        in_multi = False
        print(line)
        break
    if in_multi:
        multi_lines.append(line)

with open('scratch/gemini_round8/d_multi_mng_disasm.txt', 'w', encoding='utf-8') as f:
    f.writelines(multi_lines)

print(f"Extracted {len(multi_lines)} lines of disassembly for d_multi_mng.cpp")
