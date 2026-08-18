import re
import os

with open('scratch/gemini_round3/d_nand_thread_disasm.txt', 'r') as f:
    text = f.read()

# Let's inspect all function disassembly blocks
raw_blocks = text.split('.fn ')
fns = []
for b in raw_blocks[1:]:
    lines = b.splitlines()
    fn_name = lines[0].split(',')[0].strip()
    if not fn_name.startswith('gap_'):
        fns.append((fn_name, lines))

print(f"Total real functions: {len(fns)}")

# Collect all calls and check against symbols
with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    sym_lines = f.readlines()

all_syms = {}
for line in sym_lines:
    line = line.strip()
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);', line)
    if m:
        all_syms[m.group(1)] = (m.group(2), int(m.group(3), 16))

# Let's check which symbols are defined in banked files vs slices/wiimj2d.json
with open('slices/wiimj2d.json', 'r') as f:
    slices_json = f.read()

# Let's check syms.txt
with open('syms.txt', 'r') as f:
    syms_txt = f.read()

# Check include files for declarations
include_files = []
for root, dirs, files in os.walk('include'):
    for file in files:
        if file.endswith(('.h', '.hpp')):
            include_files.append(os.path.join(root, file))

all_calls = set()
for fn_name, lines in fns:
    for l in lines:
        m_bl = re.search(r'bl\s+([a-zA-Z0-9_@:]+)', l)
        if m_bl:
            target = m_bl.group(1)
            # ignore internal helper calls
            if target not in ['_savegpr_27', '_restgpr_27', '_savegpr_28', '_restgpr_28', '_savegpr_29', '_restgpr_29', '_savegpr_30', '_restgpr_30', '_savegpr_31', '_restgpr_31']:
                all_calls.add(target)

print("\n=== ALL EXTERNAL CALLS MADE IN d_nand_thread.cpp ===")
for c in sorted(all_calls):
    # Check if defined within d_nand_thread.cpp
    is_internal = any(c == f[0] for f in fns)
    if is_internal:
        print(f"[INTERNAL] {c}")
        continue
    
    # Check symbol in wiimj2d_symbols
    sym_info = all_syms.get(c, ("UNKNOWN", 0))
    
    # Check in slices_json (banked translation units)
    # If c is in a banked TU that is compiled from source/
    in_slices = f'"{c}"' in slices_json or c in slices_json
    
    # Check in syms.txt
    in_syms_txt = f'{c}=' in syms_txt or f'{c} =' in syms_txt
    
    # Check if declared in include headers
    decl_headers = []
    # Strip mangling for simple grep if C function, or check mangled name
    # Let's search headers for the symbol or its unmangled identifier
    simple_name = c.split('__')[0] if '__' in c else c
    for h in include_files:
        try:
            with open(h, 'r', encoding='latin-1') as hf:
                h_content = hf.read()
                if c in h_content or (simple_name in h_content and len(simple_name) > 3):
                    # Check if actually declared
                    if re.search(r'\b' + re.escape(simple_name) + r'\b', h_content):
                        decl_headers.append(os.path.relpath(h, 'include'))
        except Exception:
            pass
    
    print(f"Call: {c}")
    print(f"   ELF: {sym_info[0]} @ {hex(sym_info[1])}")
    print(f"   In syms.txt: {in_syms_txt}")
    print(f"   Matching headers ({len(decl_headers)}): {decl_headers[:3]}")
