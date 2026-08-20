"""Dump the raw (non-normalised) instruction body of one target function,
found by its function-start address, across all target text files for a unit.
Read-only; prints to stdout.

Usage: python dump_epilogue.py <unit> <addr_hex>
"""
import os
import re
import sys
import glob

ROOT = r"C:\Users\Razz\Documents\Projects\NSMBW-Decomp"
WM_UNITS = os.path.join(ROOT, 'wip', 'wm_units')
BUILD = os.path.join(ROOT, 'wip', 'return_type_sweep', 'build')

EXCLUDE_PREFIXES = ('probe', 'v', 'layout_check', 'off_probe', 'draw_probe')

CASTLE_TARGET_OBJS_TXT = glob.glob(os.path.join(BUILD, 'agent_castle', 'auto_*_text.o.txt'))


def find_target_files(unit_dir):
    out = []
    for p in glob.glob(os.path.join(unit_dir, '*.txt')):
        base = os.path.basename(p)
        if base == 'draft.txt':
            continue
        low = base.lower()
        if any(low.startswith(pfx) for pfx in EXCLUDE_PREFIXES):
            continue
        if 'compiled' in low or 'dokan_test' in low:
            continue
        out.append(p)
    return sorted(out)


def dump(unit, addr_hex):
    unit_dir = os.path.join(WM_UNITS, unit)
    paths = find_target_files(unit_dir)
    if unit == 'agent_castle':
        paths += CASTLE_TARGET_OBJS_TXT
    want = int(addr_hex, 16)
    for p in paths:
        text = open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'^\.fn (\S+?), \w+\n(.*?)^\.endfn', text, re.M | re.S):
            body = m.group(2)
            addrs = re.findall(r'/\* ([0-9A-Fa-f]{8}) ', body)
            if not addrs or int(addrs[0], 16) != want:
                continue
            print('FOUND in', os.path.basename(p), '->', m.group(1))
            for line in body.splitlines():
                if line.strip():
                    print(' ', line.rstrip())
            return
    print('NOT FOUND at', addr_hex, 'in any target file for', unit)


if __name__ == '__main__':
    dump(sys.argv[1], sys.argv[2])
