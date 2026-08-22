import subprocess, re

subprocess.run(['bin/dtk-windows-x86_64.exe', 'elf', 'disasm', 'scratch/gemini_round21/d_enemy_toride_kokoopa.o', 'scratch/gemini_round21/obj_dis.txt'], check=True)

with open('scratch/gemini_round21/obj_dis.txt', encoding='utf-8') as f:
    lines = f.readlines()

objs = []
inside = False
cur_name = None
cur_lines = []
for line in lines:
    m = re.match(r'^\.obj\s+"?([^",]+)"?,\s*(\w+)', line.strip())
    if m:
        inside = True
        cur_name = m.group(1)
        cur_lines = [line]
        continue
    if inside:
        cur_lines.append(line)
        if line.strip().startswith('.endobj'):
            inside = False
            byte_count = 0
            for l in cur_lines:
                bm = re.match(r'/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f ]+)\*/', l)
                if bm:
                    raw = bm.group(1).strip().split()
                    byte_count += len(raw)
            objs.append((cur_name, byte_count))

print(f'Total objects in draft .o: {len(objs)}')
for name, sz in objs:
    print(f'{sz:5d} B (0x{sz:04X}) : {name}')
