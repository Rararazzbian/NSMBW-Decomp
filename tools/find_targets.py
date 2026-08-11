#!/usr/bin/env python3

# find_targets.py
# Ranks undecompiled .text runs by how completely the project's headers already
# describe them. Units with complete headers match on the first try far more
# often than units needing new struct reconstruction, so this is the cheapest
# available predictor of which run to pick up next.
#
# Requires prepare_objdiff.py to have been run (reads bin/dtk/wiimj2d_symbols.txt).
#
# Usage: python tools/find_targets.py [min_size] [max_size] [min_coverage]

import glob
import io
import json
import re
import sys
from pathlib import Path

sys.path.append('tools')

from project_settings import BUILDDIR, INCDIRS, SLICEDIR

TEXT_BASE = 0x80006780

SYM_RE = re.compile(
    r'^(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)')
DECL_RE = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\(')
PLACEHOLDER_RE = re.compile(r'^(fn_|lbl_|func_)[0-9A-Fa-f]{8}$')


def load_symbols(path: Path) -> list[tuple[int, int, str]]:
    syms = []
    with io.open(path, encoding='utf-8') as f:
        for line in f:
            m = SYM_RE.match(line)
            if m:
                syms.append((int(m.group(2), 16), int(m.group(3), 16), m.group(1)))
    syms.sort()
    return syms


def load_covered(path: Path) -> list[tuple[int, int]]:
    """Address ranges already claimed by a matching decompiled slice."""
    covered = []
    with io.open(path, encoding='utf-8') as f:
        slice_file = json.load(f)
    for s in slice_file['slices']:
        rng = s.get('memoryRanges', {}).get('.text')
        if rng and s.get('source') and not s.get('nonMatching'):
            lo, hi = [int(x, 16) + TEXT_BASE for x in rng.split('-')]
            covered.append((lo, hi))
    return covered


def load_declared() -> set[str]:
    """Every identifier the project's headers declare as a function."""
    declared = set()
    for inc in INCDIRS:
        for header in glob.glob(f'{inc}/**/*.h', recursive=True):
            try:
                text = io.open(header, encoding='utf-8', errors='replace').read()
            except OSError:
                continue
            declared.update(m.group(1) for m in DECL_RE.finditer(text))
    return declared


def build_runs(syms, covered):
    """Contiguous stretches of undecompiled functions.

    A run breaks on a decompiled function or on a gap larger than the 16-byte
    function alignment CodeWarrior applies inside a translation unit.
    """
    def is_covered(addr):
        return any(lo <= addr < hi for lo, hi in covered)

    runs, cur, prev_end = [], [], None
    for addr, size, name in syms:
        if is_covered(addr) or size == 0:
            if cur:
                runs.append(cur)
            cur, prev_end = [], None
            continue
        if prev_end is not None and addr - prev_end > 16:
            runs.append(cur)
            cur = []
        cur.append((addr, size, name))
        prev_end = addr + size
    if cur:
        runs.append(cur)
    return runs


def main():
    min_size = int(sys.argv[1]) if len(sys.argv) > 1 else 200
    max_size = int(sys.argv[2]) if len(sys.argv) > 2 else 100000
    min_cov = float(sys.argv[3]) if len(sys.argv) > 3 else 0.85

    syms = load_symbols(BUILDDIR / 'dtk/wiimj2d_symbols.txt')
    covered = load_covered(SLICEDIR / 'wiimj2d.json')
    declared = load_declared()

    scored = []
    for run in build_runs(syms, covered):
        total = sum(size for _, size, _ in run)
        named = [n for _, _, n in run if not PLACEHOLDER_RE.match(n)]
        if not named:
            continue
        coverage = len([n for n in named if n in declared]) / float(len(run))
        if coverage >= min_cov and min_size <= total <= max_size:
            scored.append((coverage, total, run))
    scored.sort(key=lambda x: -x[1])

    print(f'{len(scored)} undecompiled runs '
          f'({min_size}..{max_size} bytes, header coverage >= {min_cov:.2f})\n')
    for coverage, total, run in scored:
        print(f'0x{run[0][0]:08X}  {total:5d} B  {len(run):3d} fns  '
              f'hdr-cov {coverage * 100:.0f}%')
        for addr, size, name in run[:14]:
            print(f'      0x{addr:08X} {size:5d}  {name}')
        if len(run) > 14:
            print(f'      ... +{len(run) - 14} more')
        print('')


if __name__ == '__main__':
    main()
