import json
import re
import os

with open('slices/wiimj2d.json', 'r') as f:
    slices_data = json.load(f)

with open('syms.txt', 'r') as f:
    syms_txt_lines = f.readlines()

pinned_syms = {}
for line in syms_txt_lines:
    line = line.strip()
    if '=' in line and not line.startswith('#'):
        parts = line.split('=')
        pinned_syms[parts[0].strip()] = parts[1].strip()

with open('bin/dtk/wiimj2d_symbols.txt', 'r') as f:
    sym_lines = f.readlines()

all_syms = {}
for line in sym_lines:
    line = line.strip()
    m = re.match(r'^(\S+)\s*=\s*(\.[a-zA-Z0-9_]+):(0x[0-9a-fA-F]+);\s*(?://\s*type:(\S+)\s*size:(0x[0-9a-fA-F]+))?', line)
    if m:
        name, sec, addr_s, stype, size_s = m.groups()
        all_syms[name] = {
            'sec': sec,
            'addr': int(addr_s, 16),
            'size': int(size_s, 16) if size_s else 0,
            'type': stype
        }

sec_bases = {}
for sec_name, sec_info in slices_data['meta']['sections'].items():
    sec_bases[sec_name] = int(sec_info['addr'], 16)

landed_ranges = []
for s in slices_data['slices']:
    src = s['source']
    for sec_name, r_str in s['memoryRanges'].items():
        start_off, end_off = r_str.split('-')
        start_addr = sec_bases[sec_name] + int(start_off, 16)
        end_addr = sec_bases[sec_name] + int(end_off, 16)
        landed_ranges.append((sec_name, start_addr, end_addr, src))

def find_slice(sec, addr):
    for s_sec, s_start, s_end, src in landed_ranges:
        if s_sec == sec and s_start <= addr < s_end:
            return src
    return None

calls = [
    'NANDCheck',
    'NANDClose',
    'NANDCreate',
    'NANDDelete',
    'NANDGetHomeDir',
    'NANDGetLength',
    'NANDGetType',
    'NANDInitBanner',
    'NANDMove',
    'NANDOpen',
    'NANDRead',
    'NANDSimpleSafeCancel',
    'NANDSimpleSafeClose',
    'NANDSimpleSafeOpen',
    'NANDWrite',
    'OSGetCurrentThread',
    'OSGetThreadPriority',
    'OSInitCond',
    'OSInitMutex',
    'OSLockMutex',
    'OSResumeThread',
    'OSSignalCond',
    'OSTryLockMutex',
    'OSUnlockMutex',
    'OSWaitCond',
    '__ct__Q23EGG6ThreadFUliiPQ23EGG4Heap',
    '__dl__FPv',
    '__dt__Q23EGG6ThreadFv',
    '__nw__FUl',
    'calcCRC32__4sCrcFPCvUl',
    'getMsg__10dMessage_cFUlUl',
    'getRes__6dRes_cCFPCcPCc',
    'getSaveGame__10dSaveMng_cFSc',
    'getTempGame__10dSaveMng_cFSc',
    'memcpy',
    'setCurrentHeap__5mHeapFPQ23EGG4Heap',
]

include_files = []
for root, dirs, files in os.walk('include'):
    for file in files:
        if file.endswith(('.h', '.hpp')):
            include_files.append(os.path.join(root, file))

def search_headers(sym):
    found = []
    # If C function
    c_pattern = r'\b' + re.escape(sym) + r'\b'
    # If CFront mangled, extract class / function name
    if '__' in sym:
        fn_part = sym.split('__')[0]
        c_pattern = r'\b' + re.escape(fn_part) + r'\b'
    
    for h in include_files:
        try:
            with open(h, 'r', encoding='latin-1') as f:
                content = f.read()
                if sym in content or re.search(c_pattern, content):
                    found.append(os.path.relpath(h, 'include'))
        except:
            pass
    return found

print("\n" + "="*95)
print(f"{'SYMBOL':<36} | {'CATEGORY':<12} | {'ELF ADDR':<10} | {'STATUS / DETAILS'}")
print("="*95)

for c in calls:
    info = all_syms.get(c)
    if not info:
        print(f"{c:<36} | UNKNOWN      | ???        | Not in ELF symbols!")
        continue
    
    elf_addr = hex(info['addr'])
    sl = find_slice(info['sec'], info['addr'])
    is_pinned = c in pinned_syms
    hdrs = search_headers(c)
    
    # Category classification:
    # (a) Banked TU (already compiled from source/)
    # (b) Declared in header, undecompiled -> needs syms.txt pin
    # (c) Not declared anywhere -> needs header addition + syms.txt pin
    
    if sl:
        cat = "(a) Banked"
        status = f"Banked in {sl}"
    elif len(hdrs) > 0:
        if is_pinned:
            cat = "(b) Pinned"
            status = f"Declared in {hdrs[0]} (pinned: {pinned_syms[c]})"
        else:
            cat = "(b) Unpinned"
            status = f"Declared in {hdrs[0]} (NEEDS PIN at {elf_addr})"
    else:
        cat = "(c) No Header"
        status = f"Not in headers (NEEDS HEADER DECL + PIN at {elf_addr})"
        
    print(f"{c:<36} | {cat:<12} | {elf_addr:<10} | {status}")
