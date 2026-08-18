import os, re

ROOT = r'c:\Users\Razz\Documents\Projects\NSMBW-Decomp'

# Check references in m_pad.cpp text
with open('scratch/gemini_round7/auto_03_8016F330_text.o.txt') as f:
    d1 = f.read()
with open('scratch/gemini_round7/auto_03_8016F808_text.o.txt') as f:
    d2 = f.read()

mpad_text = d1 + '\n' + d2

objects = [
    # .data
    ('__vt__Q24mTex8edit4b_c', '.data', 0x80329F60, 0x10),
    # .bss
    ('g_core__4mPad', '.bss', 0x80377F88, 0x10),
    ('@13954', '.bss', 0x80377F98, 0x0C),
    ('g_PadAdditionalData__4mPad', '.bss', 0x80377FA8, 0x60),
    ('s_WPADInfo__4mPad', '.bss', 0x80378008, 0x60),
    ('s_WPADInfoTmp__4mPad', '.bss', 0x80378068, 0x60),
    # .sbss
    ('g_padMg__4mPad', '.sbss', 0x8042A740, 0x04),
    ('g_currentCoreID__4mPad', '.sbss', 0x8042A744, 0x04),
    ('g_currentCore__4mPad', '.sbss', 0x8042A748, 0x04),
    ('g_IsConnected__4mPad', '.sbss', 0x8042A74C, 0x04),
    ('g_PadFrame__4mPad', '.sbss', 0x8042A750, 0x04),
    ('s_WPADInfoAvailable__4mPad', '.sbss', 0x8042A754, 0x04),
    ('s_GetWPADInfoInterval__4mPad', '.sbss', 0x8042A758, 0x04),
    ('s_GetWPADInfoCount__4mPad', '.sbss', 0x8042A75C, 0x04),
    # .sdata2
    ('@14502', '.sdata2', 0x8042E010, 0x04),
    ('@6616', '.sdata2', 0x8042E018, 0x04),
    ('@6617', '.sdata2', 0x8042E01C, 0x04),
    ('@6621', '.sdata2', 0x8042E020, 0x08),
    ('@6626', '.sdata2', 0x8042E028, 0x04),
    ('@6627', '.sdata2', 0x8042E02C, 0x04),
]

print("=== CHECKING DATA REFERENCES IN m_pad.cpp ===")
for name, sec, addr, size in objects:
    # Look for name in mpad_text
    clean_name = name.replace('$', '\\$').replace('@', '')
    found = False
    ref_fns = []
    # Check each function in disasm
    for line in mpad_text.splitlines():
        if name in line or (clean_name and clean_name in line):
            found = True
            ref_fns.append(line.strip())
    print(f"[{sec:8}] {name:30} (0x{addr:08X}, {hex(size):4}) : {'REFERENCED' if found else '*** UNREFERENCED ***'} ({len(ref_fns)} occurrences)")
    if found:
        for r in ref_fns[:3]:
            print(f"      {r}")
