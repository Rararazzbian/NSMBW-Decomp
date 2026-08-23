#!/usr/bin/env python3
"""Per-function instruction diff between a target and a draft disassembly.

Canonicalises the three things that differ for naming reasons only, so the
remaining diff count is a real score:

  * branch labels          .L_80080258 / .L_00000D98  ->  .L0, .L1, ...
  * mangled local symbols  fn_80080670__9dBg_ctr_cFP7mVec3_cf -> fn_80080670
  * pooled constants       "@69447_8042C168"@sda21 / "@13813"@sda21 -> @P0, ...

Labels and pool symbols are numbered independently on each side, by order of
first appearance, so an identical control-flow graph and an identical pool
access order compare equal regardless of the names.

Usage:
    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt FUNCTION [-v]
    python tools/auto_decomp/fndiff.py TARGET.txt DRAFT.txt --all
"""
import argparse
import re
import sys

FN_RE = re.compile(r'^\.fn\s+(\S+?),')
INSN_RE = re.compile(r'\*/\s*(\S.*)$')
LABEL_RE = re.compile(r'\.L_[0-9A-Fa-f]{8}')
POOL_RE = re.compile(r'"@[^"]*"')
MANGLE_RE = re.compile(r'\b(fn_[0-9A-Fa-f]{8})__\S+')


def read_functions(path):
    """Return {name: [instruction text, ...]} for every .fn block in path."""
    out = {}
    cur = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            line = line.rstrip('\n')
            m = FN_RE.match(line)
            if m:
                cur = m.group(1)
                out[cur] = []
                continue
            if line.startswith('.endfn'):
                cur = None
                continue
            if cur is None:
                continue
            m = INSN_RE.search(line)
            if m:
                out[cur].append(m.group(1).strip())
    return out


def canonical_name(name):
    """fn_80080670__9dBg_ctr_cFP7mVec3_cf -> fn_80080670"""
    return MANGLE_RE.sub(r'\1', name)


def canonicalise(body):
    """Rewrite labels and pool symbols to per-side sequential ids."""
    labels, pools = {}, {}

    def label(m):
        return labels.setdefault(m.group(0), '.L%d' % len(labels))

    def pool(m):
        return pools.setdefault(m.group(0), '@P%d' % len(pools))

    out = []
    for insn in body:
        insn = LABEL_RE.sub(label, insn)
        insn = POOL_RE.sub(pool, insn)
        insn = MANGLE_RE.sub(r'\1', insn)
        out.append(insn)
    return out


def frame_size(body):
    for insn in body:
        m = re.search(r'stwu\s+r1,\s*-(0x[0-9A-Fa-f]+)\(r1\)', insn)
        if m:
            return int(m.group(1), 16)
    return None


def saves(body):
    gpr, fpr, helper = set(), set(), None
    for insn in body:
        m = re.search(r'\bstw\s+r(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', insn)
        if m and int(m.group(1)) >= 13:
            gpr.add(int(m.group(1)))
        m = re.search(r'\bstfd\s+f(\d+),\s*0x[0-9A-Fa-f]+\(r1\)', insn)
        if m and int(m.group(1)) >= 14:
            fpr.add(int(m.group(1)))
        m = re.search(r'\b(_save(?:gpr|fpr)_\d+)', insn)
        if m:
            helper = m.group(1)
    return sorted(gpr), sorted(fpr), helper


def describe(body):
    gpr, fpr, helper = saves(body)
    fs = frame_size(body)
    return '%d words / frame %s / GPR %s / FPR %s%s' % (
        len(body),
        ('0x%X' % fs) if fs is not None else 'none',
        gpr or 'none',
        fpr or 'none',
        (' / %s' % helper) if helper else '',
    )


def compare(tgt, drf, name, verbose):
    t, d = canonicalise(tgt), canonicalise(drf)
    print('=== %s' % name)
    print('  target: %s' % describe(tgt))
    print('  draft : %s' % describe(drf))
    diffs = [(i, a, b) for i, (a, b) in enumerate(zip(t, d)) if a != b]
    if verbose:
        for i, a, b in diffs:
            print('  %4d  T: %-46s D: %s' % (i, a, b))
    extra = len(d) - len(t)
    print('  DIFFS %d%s' % (
        len(diffs),
        '' if extra == 0 else '  (+ %+d words of length difference)' % extra))
    return len(diffs) + abs(extra)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('target')
    ap.add_argument('draft')
    ap.add_argument('function', nargs='?')
    ap.add_argument('-a', '--all', action='store_true',
                    help='compare every function present on both sides')
    ap.add_argument('-v', '--verbose', action='store_true',
                    help='print each differing instruction pair')
    args = ap.parse_args()

    tgt = read_functions(args.target)
    drf = {canonical_name(k): v for k, v in read_functions(args.draft).items()}
    tgt = {canonical_name(k): v for k, v in tgt.items()}

    if args.all:
        names = [n for n in tgt if n in drf]
    elif args.function:
        names = [canonical_name(args.function)]
    else:
        ap.error('give a FUNCTION or --all')

    worst = 0
    for name in names:
        if name not in tgt:
            print('=== %s\n  NOT IN TARGET' % name)
            worst = max(worst, 1)
            continue
        if name not in drf:
            print('=== %s\n  NOT IN DRAFT' % name)
            worst = max(worst, 1)
            continue
        worst = max(worst, compare(tgt[name], drf[name], name, args.verbose))
    return 0 if worst == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
