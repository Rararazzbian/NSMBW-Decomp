"""Seed a work directory for harness.py: target disassembly + a draft stub.

    python tools/auto_decomp/prepare.py --unit dol/bases/d_a_en_lkuribo_base.cpp \
        --range 0x800331E0-0x800356D0

dtk splits the binary into bin/dtkspl/obj/auto_03_<ADDR>_text.o. This collects
every object whose start address falls inside the range, disassembles each, and
concatenates them in address order into work/<unit>/target.txt.

Run `python prepare_objdiff.py` first if bin/dtkspl/obj is missing.

CAUTION: the range is a hypothesis until proven. A TU does not end at its
__sinit -- the sFStateID_c<YourClass> template instantiations after it belong to
you too. Getting the end wrong was the single most common error this project
made; see HANDOFF.md. After preparing, sanity-check that the last function in
target.txt belongs to your class and the next one does not.
"""
import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HERE = os.path.dirname(os.path.abspath(__file__))
OBJ = os.path.join(ROOT, 'bin', 'dtkspl', 'obj')
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
SYMBOLS_TXT = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')

FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
ADDR_SUFFIX = re.compile(r'_[0-9A-Fa-f]{8}$')


def norm_name(n):
    return ADDR_SUFFIX.sub('', n.strip().strip('"'))


def list_functions(path):
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = FN_START.match(line.strip())
            if m and not m.group(1).startswith('gap_'):
                out.append(norm_name(m.group(1)))
    return out


def find_split_objects(lo, hi, unit):
    sinit_addrs = {}
    if os.path.exists(SYMBOLS_TXT):
        with open(SYMBOLS_TXT, encoding='utf-8', errors='replace') as f:
            for line in f:
                if '__sinit_' in line and '.text:' in line:
                    m = re.match(r'(__sinit_[^\s=]+)\s*=\s*\.text:(0x[0-9A-Fa-f]+)', line)
                    if m:
                        sym, addr_str = m.group(1), m.group(2)
                        sinit_addrs[sym] = int(addr_str, 16)

    # An object's extent runs from its own address to the NEXT object's address.
    # Selecting on "start falls inside the range" silently drops an object that
    # begins before the range and reaches into it -- exactly the containment vs
    # overlap bug that made tu_extent.py report nine TUs starting inside already
    # decompiled territory. Test for overlap.
    addressed = sorted(
        (int(m.group(1), 16), name)
        for name in os.listdir(OBJ)
        for m in [re.search(r'_([0-9A-Fa-f]{8})_(?:text|sinit)', name)] if m)

    hits = []
    for i, (addr, name) in enumerate(addressed):
        end = addressed[i + 1][0] if i + 1 < len(addressed) else addr + (1 << 24)
        if addr < hi and end > lo:          # overlap, not containment
            hits.append((addr, name))

    for name in os.listdir(OBJ):
        if re.search(r'_([0-9A-Fa-f]{8})_(?:text|sinit)', name):
            continue
        if name.startswith('auto_sinit_') and name.endswith('_text.o'):
            tag = name[len('auto_sinit_'):-len('_text.o')].lstrip('_')
            for sym, saddr in sinit_addrs.items():
                clean_sym = sym.replace('\\', '').replace('__sinit_', '').replace('.cpp', '')
                if tag in clean_sym or clean_sym.startswith(tag[:10]):
                    if lo <= saddr < hi:
                        if not any(name == h[1] for h in hits):
                            hits.append((saddr, name))
                            break

    hits.sort()
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--unit', required=True)
    ap.add_argument('--range', required=True, help='0xSTART-0xEND (virtual addresses)')
    ap.add_argument('--force', action='store_true', help='overwrite an existing draft.cpp')
    args = ap.parse_args()

    if not os.path.isdir(OBJ):
        sys.exit('%s missing -- run `python prepare_objdiff.py` first.' % OBJ)

    lo, hi = (int(x, 16) for x in args.range.split('-'))
    hits = find_split_objects(lo, hi, args.unit)

    if not hits:
        sys.exit('no split objects found in %s -- is the range right?' % args.range)

    slug = args.unit.replace('/', '_').replace('.cpp', '')
    work = os.path.join(HERE, 'work', slug)
    os.makedirs(work, exist_ok=True)
    target = os.path.join(work, 'target.txt')

    chunks = []
    for addr, name in hits:
        out = os.path.join(work, name + '.txt')
        p = subprocess.run([DTK, 'elf', 'disasm', os.path.join(OBJ, name), out],
                           cwd=ROOT, capture_output=True, text=True)
        if p.returncode != 0:
            print('  ! disasm failed for %s' % name)
            continue
        chunks.append('/* ---- %s (0x%08X) ---- */\n' % (name, addr) +
                      open(out, encoding='utf-8', errors='replace').read())
        print('  + %-40s 0x%08X' % (name, addr))
    if not chunks:
        sys.exit('nothing disassembled.')
    open(target, 'w', encoding='utf-8').write('\n'.join(chunks))

    fns = list_functions(target)
    draft = os.path.join(work, 'draft.cpp')
    if not os.path.exists(draft) or args.force:
        hpp = os.path.basename(args.unit).replace('.cpp', '.hpp')
        open(draft, 'w', encoding='utf-8').write(
            '#include <game/bases/%s>\n\n'
            '// Seeded by prepare.py. Replace with the real reconstruction.\n'
            '// %d functions to close -- see target.txt.\n' % (hpp, len(fns)))

    print('\ntarget.txt written: %d functions' % len(fns))
    print('work dir: %s' % work)
    print('\nnext:')
    print('  python tools/auto_decomp/harness.py --unit %s --list' % args.unit)
    print('  python tools/auto_decomp/harness.py --unit %s --fn <NAME>' % args.unit)


if __name__ == '__main__':
    main()

