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
MANGLE_RE = re.compile(r'\b(fn_[0-9A-Fa-f]{8})__\S+')

# A relocated operand: the symbol part of  SYMBOL@ha / @l / @sda21 / @sda2.
RELOC_RE = re.compile(r'("@[^"]*"|[\w.$]+)@(ha|l|sda21|sda2)\b')

# dtk's disambiguating retail-address suffix on an otherwise identical symbol.
ADDR_SUFFIX_RE = re.compile(r'_8[0-9A-Fa-f]{7}(?=")')

# Symbols whose *name* is generated rather than written by a human. Two of
# these that disagree across a pair are a naming artifact, not a code
# difference: the target names data after its retail address and the draft
# names it after a section offset or a source identifier.
ANON_RES = [
    re.compile(r'^"@\d+'),                     # "@69447_8042C168"  pooled float
    re.compile(r'^@\d+$'),                     # @13813
    re.compile(r'^\.\.\.\w+\.\d+$'),           # ...bss.0  ...data.0
    re.compile(r'^lbl_[0-9A-Fa-f]{8}$'),       # lbl_802F0C80
    re.compile(r'^gap_\w+$'),
    re.compile(r'_8[0-9A-Fa-f]{7}$'),          # dtk address suffix
]


def is_anon(sym):
    return any(r.search(sym) for r in ANON_RES)


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
    """Rewrite branch labels to per-side sequential ids and strip manglings.

    Relocated operands are deliberately left alone here — they are compared
    pairwise by artifact_pair() instead, because renumbering them per side
    desynchronises the moment one side names a datum and the other does not.
    """
    labels = {}

    def label(m):
        return labels.setdefault(m.group(0), '.L%d' % len(labels))

    out = []
    for insn in body:
        insn = LABEL_RE.sub(label, insn)
        insn = MANGLE_RE.sub(r'\1', insn)
        # dtk appends the retail address to symbols it must disambiguate, e.g.
        #   bl "baseID_Jump<10sStateID_c>__Fv_RC12sStateIDIf_c_800A8720"
        # against a draft that emits the same template instantiation unadorned.
        insn = ADDR_SUFFIX_RE.sub('', insn)
        out.append(insn)
    return out


def artifact_pair(a, b):
    """True if a and b differ only in generated symbol names."""
    if a == b:
        return False
    sa = RELOC_RE.findall(a)
    sb = RELOC_RE.findall(b)
    if not sa or len(sa) != len(sb):
        return False
    for (na, _), (nb, _) in zip(sa, sb):
        if na == nb:
            continue
        # A real symbol named differently on both sides is a real difference.
        if not (is_anon(na) or is_anon(nb)):
            return False
    return RELOC_RE.sub(r'@SYM@\2', a) == RELOC_RE.sub(r'@SYM@\2', b)


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
    diffs, artifacts = [], []
    for i, (a, b) in enumerate(zip(t, d)):
        if a == b:
            continue
        (artifacts if artifact_pair(a, b) else diffs).append((i, a, b))
    if verbose:
        for i, a, b in diffs:
            print('  %4d  T: %-46s D: %s' % (i, a, b))
        for i, a, b in artifacts:
            print('  %4d  T: %-46s D: %s   [naming artifact]' % (i, a, b))
    extra = len(d) - len(t)
    print('  DIFFS %d%s%s' % (
        len(diffs),
        '' if not artifacts else '  (+ %d naming artifact(s))' % len(artifacts),
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
