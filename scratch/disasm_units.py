import subprocess
import glob
import os
import re

dtk = os.path.abspath('bin/dtk-windows-x86_64.exe')

# Disassemble all m_pad and coin_main objects
os.makedirs('scratch/disasm', exist_ok=True)

objs = glob.glob('bin/dtkspl/obj/*8016F*') + glob.glob('bin/dtkspl/obj/*m_pad*') + glob.glob('bin/dtkspl/obj/*80027*') + glob.glob('bin/dtkspl/obj/*coin_m*')
print("Objects to disassemble:")
for obj in objs:
    print(" ", obj)
    out_txt = os.path.join('scratch/disasm', os.path.basename(obj) + '.txt')
    cmd = [dtk, 'elf', 'disasm', os.path.abspath(obj), out_txt]
    subprocess.run(cmd, check=True)
    print(f"  Disassembled to {out_txt}")

