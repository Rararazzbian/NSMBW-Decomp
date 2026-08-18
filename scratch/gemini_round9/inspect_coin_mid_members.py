import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
disasm_path = ROOT / 'scratch' / 'gemini_round9' / 'coin_main_disasm.txt'
lines = disasm_path.read_text().splitlines()

curr_fn = None
for line in lines:
    m_fn = re.match(r'^\.fn\s+([^,]+)', line.strip())
    if m_fn:
        curr_fn = m_fn.group(1).strip()
    
    # check for accesses in 0x690..0x750
    m_mem = re.search(r'0x(6[9a-fA-F0-9]{2}|7[0-4][a-fA-F0-9])', line)
    if m_mem:
        print(f"[{curr_fn}] {line.strip()}")
