"""A7 DIAGNOSTIC: is the mBaseSpeed/const FPR permutation flag-sensitive?

Compiles the UNMODIFIED wip/gapA/gapA_all.cpp with one flag varied at a time and
reports (a) whether the compile was accepted, (b) the instruction count of
executeState_Left30Left, (c) whether indices 2/10/33/34 flipped to retail order,
and (d) how many functions in the whole unit still match.

Command line is derived from tools/auto_decomp/harness.py (compiler path,
flags_for('wiimj2d'), INCLUDES, cwd=ROOT) -- only the one flag under test moves.
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

SRC = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
FN = 'executeState_Left30Left__10dLineMng_cFv'
KEY = [2, 10, 33, 34]


def parse(path):
    fns, cur = {}, None
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'\s*\.fn\s+([^\s,]+)', line)
        if m:
            cur = m.group(1).strip('"')
            fns[cur] = []
            continue
        if re.match(r'\s*\.endfn', line):
            cur = None
            continue
        if cur is not None:
            mi = re.match(r'/\* [0-9A-F]+\s+[0-9A-F]+\s+([0-9A-F ]+?)\s*\*/\s*(.*)', line)
            if mi:
                fns[cur].append((mi.group(1).strip(), mi.group(2).strip()))
    return fns


BASE = harness.flags_for('wiimj2d')          # ['-c', ...core..., '-DREVOLUTION', '-I-']


def sub(flags, name, new):
    """Replace `-name <val>` with `-name <new>` (or drop it if new is None)."""
    out, i = [], 0
    while i < len(flags):
        if flags[i] == name:
            if new is not None:
                out += [name, new]
            i += 2
        else:
            out.append(flags[i])
            i += 1
    return out


def solo(flags, old, new):
    """Replace a single valueless token (e.g. -O4 -> -O3)."""
    return [new if f == old else f for f in flags]


VARIANTS = []


def V(label, flags):
    VARIANTS.append((label, flags))


V('BASELINE (project flags)', list(BASE))
# --- optimisation level / mode
for o in ('-O4,p', '-O4,s', '-O3', '-O2', '-O1', '-O0', '-O4,speed', '-O4,space'):
    V('opt %s' % o, solo(BASE, '-O4', o))
# --- scheduling
V('-sched on', BASE + ['-sched', 'on'])
V('-sched off', BASE + ['-sched', 'off'])
V('-opt schedule', BASE + ['-opt', 'schedule'])
V('-opt noschedule', BASE + ['-opt', 'noschedule'])
V('-opt level=4,schedule', BASE + ['-opt', 'level=4,schedule'])
V('-opt no_peephole', BASE + ['-opt', 'nopeephole'])
V('-opt peephole', BASE + ['-opt', 'peephole'])
V('-opt nolifetimes', BASE + ['-opt', 'nolifetimes'])
V('-opt lifetimes', BASE + ['-opt', 'lifetimes'])
V('-opt nodeadcode', BASE + ['-opt', 'nodeadcode'])
V('-opt global', BASE + ['-opt', 'global'])
V('-opt noglobal', BASE + ['-opt', 'noglobal'])
V('-opt noloop', BASE + ['-opt', 'noloop'])
V('-opt loop', BASE + ['-opt', 'loop'])
V('-opt nostrength', BASE + ['-opt', 'nostrength'])
V('-opt nopropagation', BASE + ['-opt', 'nopropagation'])
V('-opt nocse', BASE + ['-opt', 'nocse'])
# --- float
for fp in ('fmadd', 'fastmath', 'soft', 'off', 'single', 'double', 'hard,fmadd'):
    V('-fp %s' % fp, sub(BASE, '-fp', fp))
V('-fp_contract on', BASE + ['-fp_contract', 'on'])
V('-fp_contract off', BASE + ['-fp_contract', 'off'])
V('-nofmadds', BASE + ['-nofmadds'])
V('-ffreestanding-ish -maf off', BASE + ['-maf', 'off'])
V('-maf on', BASE + ['-maf', 'on'])
# --- inlining
for il in ('auto', 'on', 'off', 'all', 'deferred', 'noauto,level=1'):
    V('-inline %s' % il, sub(BASE, '-inline', il))
# --- ipa
for ip in ('function', 'off', 'program', 'file,inline'):
    V('-ipa %s' % ip, sub(BASE, '-ipa', ip))
# --- small data
V('-sdata 0', BASE + ['-sdata', '0'])
V('-sdata2 0', BASE + ['-sdata2', '0'])
V('-sdata 0 -sdata2 0', BASE + ['-sdata', '0', '-sdata2', '0'])
V('-sdata 8', BASE + ['-sdata', '8'])
V('-sdata2 8', BASE + ['-sdata2', '8'])
V('-sdata 64 -sdata2 64', BASE + ['-sdata', '64', '-sdata2', '64'])
# --- misc codegen
V('-use_lmw_stmw on', BASE + ['-use_lmw_stmw', 'on'])
V('-use_lmw_stmw off', BASE + ['-use_lmw_stmw', 'off'])
V('-func_align 4', BASE + ['-func_align', '4'])
V('-func_align 32', BASE + ['-func_align', '32'])
V('-common on', BASE + ['-common', 'on'])
V('-common off', BASE + ['-common', 'off'])
V('-char signed', BASE + ['-char', 'signed'])
V('-char unsigned', BASE + ['-char', 'unsigned'])
V('-str reuse', BASE + ['-str', 'reuse'])
V('-rostr', BASE + ['-rostr'])
V('-align powerpc', BASE + ['-align', 'powerpc'])
V('-proc 750', solo(BASE, 'gekko', '750'))
V('-proc broadway', solo(BASE, 'gekko', 'broadway'))
V('-volatileasm', BASE + ['-volatileasm'])
V('-schedule on(alias)', BASE + ['-schedule', 'on'])
V('-Cpp_exceptions on', sub(BASE, '-Cpp_exceptions', 'on'))
V('-RTTI on', sub(BASE, '-RTTI', 'on'))
V('-enum min', sub(BASE, '-enum', 'min'))


def compile_with(flags, obj):
    args = [harness.MWCC] + flags + [SRC, '-o', obj]
    for inc in [INC] + harness.INCLUDES:
        args += ['-i', inc.replace('/', os.sep)]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')


def main():
    tgt_all = parse(TARGET)
    tgt = tgt_all[FN]
    tgt_txt = [t for _, t in tgt]
    tgt_can = harness.canonicalise(tgt_txt)
    print('target %s: %d instructions' % (FN, len(tgt)))
    print('key lines (retail):')
    for i in KEY:
        print('  %3d  %s' % (i, tgt_txt[i]))
    print()

    # whole-unit baseline set: which target functions match under BASELINE
    rows = []
    obj = os.path.join(HERE, 'v.o')
    txt = os.path.join(HERE, 'v.txt')
    for label, flags in VARIANTS:
        ok, log = compile_with(flags, obj)
        if not ok:
            err = next((l.strip() for l in log.splitlines() if 'rror' in l or 'nrecognized' in l
                        or 'llegal' in l), (log.strip().splitlines() or ['?'])[0])
            rows.append((label, 'REJECTED', '-', '-', '-', err[:70]))
            continue
        dok, dlog = harness.disasm(obj, txt)
        if not dok:
            rows.append((label, 'yes', 'disasm-fail', '-', '-', dlog[:60]))
            continue
        got_all = parse(txt)
        got = got_all.get(FN)
        if not got:
            rows.append((label, 'yes', 'fn-missing', '-', '-', ''))
            continue
        got_txt = [t for _, t in got]
        n = len(got)
        # key-line comparison, canonicalised so @sda21 symbol naming cannot leak in
        g_can = harness.canonicalise(got_txt)
        if n == len(tgt):
            flipped = all(g_can[i] == tgt_can[i] for i in KEY)
            keystate = 'FLIPPED->retail' if flipped else 'same as draft'
            if not flipped:
                # is it even the same *set* of four lines, just permuted?
                pass
        else:
            keystate = 'n/a (len %d)' % n
        full = 'EXACT' if g_can == tgt_can else 'differs'
        # whole-unit: count matching functions
        nm = 0
        tot = 0
        for name, body in tgt_all.items():
            if name.startswith('gap_'):
                continue
            tot += 1
            g = got_all.get(name)
            if g and harness.canonicalise([t for _, t in g]) == \
                     harness.canonicalise([t for _, t in body]):
                nm += 1
        rows.append((label, 'yes', str(n), keystate, full, '%d/%d unit' % (nm, tot)))

    w = max(len(r[0]) for r in rows)
    print('%-*s  %-9s %-11s %-16s %-8s %s' % (w, 'FLAG VARIED', 'ACCEPTED', 'INSNS',
                                              'KEY 2/10/33/34', 'FN', 'NOTE'))
    for r in rows:
        print('%-*s  %-9s %-11s %-16s %-8s %s' % (w, r[0], r[1], r[2], r[3], r[4], r[5]))


if __name__ == '__main__':
    main()
