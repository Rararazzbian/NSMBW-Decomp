import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SIBMAP = ROOT / 'tools' / 'sibmap.py'

print("Building sibmap disassembly cache...")
p_dis = subprocess.run([sys.executable, str(SIBMAP), 'disasm'], cwd=str(ROOT), capture_output=True, text=True)
print("disasm returncode:", p_dis.returncode)
if p_dis.stderr:
    print("disasm stderr:", p_dis.stderr)

print("\nRunning sibmap map on d_a_en_coin_main...")
cmd = [
    sys.executable, str(SIBMAP), 'map',
    '--lo', '0x800272F0',
    '--hi', '0x800281C0',
    '--objs', 'bin/dtkspl/obj/auto_03_800272F0_text.o',
    '--out', str(ROOT / 'scratch' / 'gemini_round9')
]

p_map = subprocess.run(cmd, cwd=str(ROOT), capture_output=True, text=True)
print("map returncode:", p_map.returncode)
print("map stderr:", p_map.stderr)
print("map stdout (first 2000 chars):", p_map.stdout[:2000])
