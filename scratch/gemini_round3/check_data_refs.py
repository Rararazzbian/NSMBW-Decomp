import re

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    lines = f.readlines()

sda_refs = set()
ha_l_refs = set()

for l in lines:
    m_sda = re.findall(r'(\S+)@(sda21|sda)', l)
    for sym, kind in m_sda:
        sda_refs.add((sym, kind))
    m_hal = re.findall(r'(\S+)@(ha|l)', l)
    for sym, kind in m_hal:
        ha_l_refs.add(sym)

print("=== SDA / SDA21 REFERENCES ===")
for sym, kind in sorted(sda_refs):
    print(f"  {sym} (@{kind})")

print("\n=== HA / L REFERENCES ===")
for sym in sorted(ha_l_refs):
    print(f"  {sym}")
