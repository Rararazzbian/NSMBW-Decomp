import re

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    lines = f.readlines()

# Let's inspect functions and symbols referenced
current_fn = None
fn_data = {}
all_refs = set()

for line in lines:
    m_fn = re.match(r'^\s*//\s*(\S+)\s*\((0x[0-9a-fA-F]+)\)', line)
    if m_fn:
        current_fn = m_fn.group(1)
        fn_data[current_fn] = []
        continue
    if current_fn:
        fn_data[current_fn].append(line)
        # Check relocations or symbol references
        # Typically lines have comments with target symbols or instructions
        m_rel = re.findall(r'/\*\s*([0-9a-fA-F]+:\s*R_[A-Za-z0-9_]+\s+[^*/]+)\*/', line)
        for r in m_rel:
            all_refs.add(r.strip())
        # Also check bl instructions
        m_bl = re.search(r'bl\s+([a-zA-Z0-9_@:]+)', line)
        if m_bl:
            all_refs.add("CALL: " + m_bl.group(1))

print(f"Total functions in disasm: {len(fn_data)}")
for fn, lns in fn_data.items():
    print(f"Function: {fn} ({len(lns)} lines)")

print("\n=== All Relocations / Branch targets ===")
for r in sorted(all_refs):
    print(r)
