import os
import json
import re

with open('slices/wiimj2d.json', 'r') as f:
    slices = json.load(f)

# Find all symbols in banked slices
banked_symbols = set()
for slice_name, slice_info in slices.items():
    if 'syms' in slice_info:
        for sym_name, sym_addr in slice_info['syms'].items():
            banked_symbols.add(sym_name)

# Also check source/ files for matching functions
source_functions = set()
for root, dirs, files in os.walk('source'):
    for file in files:
        if file.endswith(('.cpp', '.c')):
            with open(os.path.join(root, file), 'r', encoding='latin-1') as f:
                content = f.read()
                # find functions
                matches = re.findall(r'(\w+::\w+)\s*\(', content)
                for m in matches:
                    source_functions.add(m)

print(f"Banked symbols in slices: {len(banked_symbols)}")

# Let's inspect the exact list of external calls from d_nand_thread.cpp
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

with open('syms.txt', 'r') as f:
    syms_txt_lines = f.readlines()
pinned_syms = set()
for line in syms_txt_lines:
    line = line.strip()
    if '=' in line and not line.startswith('#'):
        pinned_syms.add(line.split('=')[0].strip())

# Check each call
print("\n=== CLASSIFICATION OF ALL 24 EXTERNAL CALLS ===")
for c in calls:
    is_banked = c in banked_symbols
    is_pinned = c in pinned_syms
    print(f"{c}:")
    print(f"  Banked: {is_banked}, Pinned: {is_pinned}")
