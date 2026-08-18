import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
lines = disasm_path.read_text().splitlines()

bl_calls = []
data_refs = []

for line in lines:
    m_bl = re.search(r'bl\s+([^\s]+)', line)
    if m_bl:
        bl_calls.append(m_bl.group(1))
        
    m_data = re.search(r'([A-Za-z_@][A-Za-z0-9_@]+)@(ha|l|sdarx|sda21)', line)
    if m_data:
        data_refs.append(m_data.group(1))

unique_calls = sorted(set(bl_calls))
unique_data = sorted(set(data_refs))

print(f"=== UNIQUE CALLEES ({len(unique_calls)}) ===")
for c in unique_calls:
    print(f"  {c}")

print(f"\n=== UNIQUE DATA SYMBOLS ({len(unique_data)}) ===")
for d in unique_data:
    print(f"  {d}")
