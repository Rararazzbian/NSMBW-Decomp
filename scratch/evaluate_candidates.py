import sys
import os
import re
import json
import subprocess
from pathlib import Path
from collections import defaultdict

sys.path.append('tools')
import sibmap
from elffile import ElfFile

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
DIS = os.path.join(ROOT, 'tools', 'dis')

corpus = sibmap.load_corpus()

# Load symbols from d_basesNP_symbols.txt
symbols_by_sec = defaultdict(list)
with open('bin/dtk/d_basesNP_symbols.txt') as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        m = re.match(r'^([^=]+)=\s*([^:]+):(0x[0-9a-fA-F]+);\s*//\s*(.*)$', line)
        if m:
            name, sname, addr, comment = m.groups()
            size_m = re.search(r'size:(0x[0-9a-fA-F]+)', comment)
            size = int(size_m.group(1), 16) if size_m else 0
            symbols_by_sec[sname.strip()].append({
                'name': name.strip(),
                'sec': sname.strip(),
                'addr': int(addr, 16),
                'size': size,
                'comment': comment
            })

for sec in symbols_by_sec:
    symbols_by_sec[sec].sort(key=lambda s: s['addr'])

# Read d_basesNP.rel text section
with open('original/d_basesNP.rel', 'rb') as f:
    rel_bytes = f.read()

# Section 1 is .text at offset 0xF0
text_data = rel_bytes[0xF0 : 0xF0 + 0x1C6004]

def get_fn_words(addr, size):
    words = []
    for off in range(0, size, 4):
        w = int.from_bytes(text_data[addr + off : addr + off + 4], 'big')
        words.append(w)
    return words

candidates = [
    {
        'name': 'd_a_wm_ghost.cpp',
        'profile': 'g_profile_WM_GHOST',
        'class': 'daWmGhost_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x163620, 0x164230),
        'data': (0x44A9C, 0x44CB4),
        'rodata': (0x8880, 0x88B8),
        'ctors': (0x3E0, 0x3E4),
        'bss': (0xFDC0, 0xFDD0)
    },
    {
        'name': 'd_a_wm_grid.cpp',
        'profile': 'g_profile_WM_GRID',
        'class': 'daWmGrid_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x164230, 0x164430),
        'data': (0x44CB4, 0x44D54),
        'rodata': (0x88B8, 0x88D0),
        'ctors': (0x3E4, 0x3E8),
        'bss': (0xFDD0, 0xFDE0)
    },
    {
        'name': 'd_a_wm_kinoko_base.cpp',
        'profile': 'g_profile_WM_KINOKO_BASE',
        'class': 'daWmKinokoBase_c',
        'base_class': 'dWmObjActor_c',
        'is_base': True,
        'unblocks': 'daWmKinoko1up_c, daWmKinokoRed_c, daWmKinokoStar_c (3 derived TUs)',
        'text': (0x16B2D0, 0x16BDA0),
        'data': (0x458E4, 0x45AB4),
        'rodata': (0x8B70, 0x8BA8),
        'ctors': (0x3FC, 0x400),
        'bss': (0xFE88, 0xFEA0)
    },
    {
        'name': 'd_a_wm_kinoko_1up.cpp',
        'profile': 'g_profile_WM_KINOKO_1UP',
        'class': 'daWmKinoko1up_c',
        'base_class': 'daWmKinokoBase_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x16B0F0, 0x16B2D0),
        'data': (0x457EC, 0x458E4),
        'rodata': (0x8B58, 0x8B70),
        'ctors': (0x3F8, 0x3FC),
        'bss': (0xFE78, 0xFE88)
    },
    {
        'name': 'd_a_wm_boss_base.cpp',
        'profile': 'g_profile_WM_BOSS_BASE',
        'class': 'daWmBossBase_c',
        'base_class': 'dWmDemoActor_c',
        'is_base': True,
        'unblocks': 'daWmBossIggy_c, daWmBossLarry_c, daWmBossLemmy_c, daWmBossLudwig_c, daWmBossMorton_c, daWmBossRoy_c, daWmBossWendy_c (7 derived TUs)',
        'text': (0x189AC0, 0x18A260),
        'data': (0x485FC, 0x488C8),
        'rodata': (0x9590, 0x95D8),
        'ctors': (0x454, 0x458),
        'bss': (0x10B48, 0x10B60)
    },
    {
        'name': 'd_a_wm_boss_larry.cpp',
        'profile': 'g_profile_WM_BOSS_LARRY',
        'class': 'daWmBossLarry_c',
        'base_class': 'daWmBossBase_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x18B470, 0x18B690),
        'data': (0x48CD8, 0x48E58),
        'rodata': (0x96B8, 0x96D0),
        'ctors': (0x464, 0x468),
        'bss': (0x10BD0, 0x10BE0)
    },
    {
        'name': 'd_a_wm_smallcloud.cpp',
        'profile': 'g_profile_WM_SMALLCLOUD',
        'class': 'daWmSmallCloud_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x1797E0, 0x179FF0),
        'data': (0x4728C, 0x47484),
        'rodata': (0x8F58, 0x8FA0),
        'ctors': (0x430, 0x434),
        'bss': (0x10130, 0x10140)
    },
    {
        'name': 'd_a_wm_tower.cpp',
        'profile': 'g_profile_WM_TOWER',
        'class': 'daWmTower_c',
        'base_class': 'dWmObjActor_c',
        'is_base': False,
        'unblocks': 'None (leaf)',
        'text': (0x185710, 0x185B70),
        'data': (0x480B4, 0x4818C),
        'rodata': (0x9488, 0x94A0),
        'ctors': (0x44C, 0x450),
        'bss': (0x10A98, 0x10AA8)
    }
]

print("=== EVALUATING 8 CANDIDATES IN d_basesNP ===")
for cand in candidates:
    t_start, t_end = cand['text']
    fns = [s for s in symbols_by_sec['.text'] if t_start <= s['addr'] < t_end]
    code_bytes = sum(s['size'] for s in fns)
    span_bytes = t_end - t_start
    
    total_exact_match_bytes = 0
    total_shape_match_bytes = 0
    total_target_bytes = 0
    
    fn_reports = []
    for fn in fns:
        f_addr = fn['addr']
        f_size = fn['size']
        if f_size == 0:
            continue
        words = get_fn_words(f_addr, f_size)
        f_shape = [sibmap.shape(w) for w in words]
        f_sig = sibmap.opsig(words)
        
        # Find best match in corpus
        best_exact_score = 0.0
        best_shape_score = 0.0
        best_corpus_fn = None
        
        for c in corpus:
            # check sig similarity filter
            if sibmap.sigsim(f_sig, c['sig']) < 0.2:
                continue
            
            # shape similarity
            s_score = sibmap.sim(f_shape, c['shape'])
            if s_score > best_shape_score:
                best_shape_score = s_score
                best_corpus_fn = c
            
            # exact similarity
            e_score = sibmap.sim(words, c['words'])
            if e_score > best_exact_score:
                best_exact_score = e_score
        
        total_target_bytes += f_size
        total_exact_match_bytes += best_exact_score * f_size
        total_shape_match_bytes += best_shape_score * f_size
        
        fn_reports.append((fn['name'], f_size, best_exact_score, best_shape_score, best_corpus_fn['name'] if best_corpus_fn else 'None', best_corpus_fn['tu'] if best_corpus_fn else 'None'))

    exact_pct = (total_exact_match_bytes / total_target_bytes * 100) if total_target_bytes else 0
    shape_pct = (total_shape_match_bytes / total_target_bytes * 100) if total_target_bytes else 0
    
    print(f"\n-------------------------------------------------------")
    print(f"Candidate: {cand['name']} ({cand['class']})")
    print(f"  Base Class: {cand['base_class']} | Is Base: {cand['is_base']}")
    print(f"  Unblocks: {cand['unblocks']}")
    print(f"  .text: 0x{t_start:06x}..0x{t_end:06x} (Span: {span_bytes} B, Code: {code_bytes} B, {len(fns)} functions)")
    print(f"  .data: 0x{cand['data'][0]:05x}..0x{cand['data'][1]:05x} (Size: 0x{cand['data'][1]-cand['data'][0]:x})")
    print(f"  .rodata: 0x{cand['rodata'][0]:04x}..0x{cand['rodata'][1]:04x} (Size: 0x{cand['rodata'][1]-cand['rodata'][0]:x})")
    print(f"  .ctors: 0x{cand['ctors'][0]:03x}..0x{cand['ctors'][1]:03x} (Size: 0x{cand['ctors'][1]-cand['ctors'][0]:x})")
    print(f"  .bss: 0x{cand['bss'][0]:04x}..0x{cand['bss'][1]:04x} (Size: 0x{cand['bss'][1]-cand['bss'][0]:x})")
    print(f"  Sibling Correspondence: {exact_pct:.2f}% exact / {shape_pct:.2f}% shape")
    print(f"  Sample functions:")
    for fname, fsize, es, ss, cname, ctu in fn_reports[:5]:
        print(f"    {fname} ({fsize} B): exact={es*100:.1f}%, shape={ss*100:.1f}% -> {ctu}::{cname}")

