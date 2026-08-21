"""Gap A residual measurement -- read-only.

Compiles wip/gapA/gapA_all.cpp ONCE, disassembles once, then for every function
present in both the draft and retail reports:
  * instruction counts (draft vs target)
  * raw-byte equality
  * canonical-text equality (harness.canonicalise)
  * REAL differences, after filtering out compiler-pool symbol names and local
    branch labels (both of which are named differently on the two sides and are
    pure noise)
  * a classification of the residual

Writes nothing outside this directory.
"""
import os, re, sys, json

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness

SRC = os.path.join(ROOT, 'wip', 'gapA', 'gapA_all.cpp')
INC = os.path.join(ROOT, 'wip', 'fix_bigtwo', 'shadow_include')
TARGET = os.path.join(ROOT, 'wip', 'line_mng_shared', 'target.txt')
OBJ = os.path.join(HERE, 'a5.o')
TXT = os.path.join(HERE, 'a5.txt')

# ---------------------------------------------------------------- parsing
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
FN_END = re.compile(r'^\.endfn\b')
INSN = re.compile(r'^/\*\s*(\S+)\s+(\S+)\s+((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})\s*\*/\s*(\S.*)$')


def parse(path):
    """{normalised name: [(addr, bytes_hex, text), ...]}"""
    fns, cur = {}, None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                cur = harness.norm_name(m.group(1))
                fns.setdefault(cur, [])
                continue
            if FN_END.match(s):
                cur = None
                continue
            if cur is not None:
                mi = INSN.match(s)
                if mi:
                    fns[cur].append((mi.group(1), ''.join(mi.group(3).split()).upper(),
                                     mi.group(4).strip()))
    return fns


# ---------------------------------------------------------------- noise filter
# Compiler-pool literal / data symbols. Retail names them "@56309_8042CBA8"
# (pool number + dtk address suffix); a fresh object names the same slot "@7869".
# Neither number is comparable across sides, so number them per-side by first
# appearance: that keeps "same literal twice" distinct from "two literals" while
# erasing the naming.
#
# Three spellings of the SAME anonymous slot have to land in one namespace,
# or the per-side numbering drifts by one and every later reference in the
# function reads as a difference:
#   retail   "@55305_8042CB5C"   pool number + dtk address suffix
#   ours     "@7870"             bare pool number
#   ours     ...bss.0            dtk's name for an unnamed section object,
#                                where retail happened to have a pool name
POOL = re.compile(r'"?@\d+(?:_[0-9A-Fa-f]{8})?"?'
                  r'|\.\.\.(?:data|rodata|bss|sbss|sdata2?)\.\d+')
# Local branch labels: retail names them by absolute address, ours by section
# offset. Same treatment.
LABEL = re.compile(r'\.L_[0-9A-Fa-f]{8}\b')


def denoise(texts):
    pool, lbl = {}, {}
    out = []
    for t in texts:
        t = POOL.sub(lambda m: pool.setdefault(m.group(0), 'SYM%d' % len(pool)), t)
        t = LABEL.sub(lambda m: lbl.setdefault(m.group(0), 'LBL%d' % len(lbl)), t)
        out.append(t)
    return out


def canon(body):
    """Exactly harness's gate: canonicalise() over the instruction texts, with
    local branches replaced by their raw (PC-relative, relocation-free) word so
    control flow is compared rather than erased."""
    lines = []
    for _, word, text in body:
        if harness.LOCAL_BRANCH.search(text):
            lines.append('%s |%s|' % (text, word))
        else:
            lines.append(text)
    return harness.canonicalise(lines)


LFS = re.compile(r'^lfs\s+f(\d+),\s*(.*)$')


def classify(pairs):
    """pairs: [(idx, target_line, draft_line)] of REAL differences."""
    if not pairs:
        return 'IDENTICAL'
    # Is every differing line on both sides an lfs into f0 or f1?
    ok = True
    for _, t, d in pairs:
        mt, md = LFS.match(t), LFS.match(d)
        if not mt or not md or mt.group(1) not in ('0', '1') or md.group(1) not in ('0', '1'):
            ok = False
            break
    if not ok:
        return 'OTHER'

    def swap01(line):
        m = LFS.match(line)
        return 'lfs f%s, %s' % ('1' if m.group(1) == '0' else '0', m.group(2))

    if all(swap01(d) == t for _, t, d in pairs):
        return 'F0/F1 SWAP (positional)'
    ts = sorted(t for _, t, _ in pairs)
    ds = sorted(swap01(d) for _, _, d in pairs)
    if ts == ds:
        return 'F0/F1 SWAP + reorder'
    ts2 = sorted(t for _, t, _ in pairs)
    ds2 = sorted(d for _, _, d in pairs)
    if ts2 == ds2:
        return 'REORDER ONLY (same lines, different order)'
    return 'OTHER (all lfs f0/f1 but not a clean swap)'


def main():
    ok, log = harness.compile_draft(SRC, OBJ, extra_inc=[INC])
    if not ok:
        sys.exit('COMPILE FAILED\n' + log[-3000:])
    ok, log = harness.disasm(OBJ, TXT)
    if not ok:
        sys.exit('DISASM FAILED\n' + log[-2000:])

    tgt, drf = parse(TARGET), parse(TXT)

    # Our draft gives real (mangled) names to functions dtk could only name by
    # address in retail. Without this map they read as NOT EMITTED -- including
    # fn_800C31C0, which is the NINTH function the Gap A fix touched.
    for dname in list(drf):
        m = re.match(r'(?:setArrElem_)?(fn_[0-9A-Fa-f]{8})__F', dname)
        if m and m.group(1) in tgt and m.group(1) not in drf:
            drf[m.group(1)] = drf[dname]
    if '__sinit_\\gapA_all_cpp' in drf:
        drf.setdefault('__sinit_\\d_line_mng_cpp', drf['__sinit_\\gapA_all_cpp'])

    # Function order in retail, restricted to those we also emit.
    order = [n for n in harness.list_functions(TARGET)]
    rows = []
    for name in order:
        if name not in drf:
            rows.append((name, len(tgt.get(name, [])), None, None, None, 'NOT EMITTED', []))
            continue
        T, D = tgt[name], drf[name]
        byte_eq = [b for _, b, _ in T] == [b for _, b, _ in D]
        canon_eq = canon(T) == canon(D)
        ft, fd = denoise([t for _, _, t in T]), denoise([t for _, _, t in D])
        pairs = [(i,
                  ft[i] if i < len(ft) else '<none>',
                  fd[i] if i < len(fd) else '<none>')
                 for i in range(max(len(ft), len(fd)))
                 if (ft[i] if i < len(ft) else '<none>') != (fd[i] if i < len(fd) else '<none>')]
        rows.append((name, len(T), len(D), byte_eq, canon_eq, classify(pairs), pairs))

    out = open(os.path.join(HERE, 'report.txt'), 'w', encoding='utf-8')

    def emit(s=''):
        print(s)
        out.write(s + '\n')

    emit('%-52s %5s %5s %5s %5s %6s  %s' % ('FUNCTION', 'TGT', 'DRF', 'BYTE', 'CANON', 'DIFFS', 'CLASS'))
    emit('-' * 130)
    n_byte = n_canon = 0
    for name, lt, ld, be, ce, cls, pairs in rows:
        if be:
            n_byte += 1
        if ce:
            n_canon += 1
        emit('%-52s %5s %5s %5s %5s %6s  %s' % (
            name[:52], lt, '-' if ld is None else ld,
            '' if be is None else ('yes' if be else 'no'),
            '' if ce is None else ('yes' if ce else 'no'),
            '-' if ld is None else len(pairs), cls))
    emit()
    emit('TOTAL: %d functions in retail; byte-equal %d; canonical-equal %d; union %d'
         % (len(rows), n_byte, n_canon,
            sum(1 for r in rows if r[3] or r[4])))

    emit()
    emit('=' * 130)
    emit('DETAIL for every function with a non-empty REAL residual')
    emit('=' * 130)
    for name, lt, ld, be, ce, cls, pairs in rows:
        if not pairs:
            continue
        emit()
        emit('%s   target=%s draft=%s   %d real diffs   %s' % (name, lt, ld, len(pairs), cls))
        for i, t, d in pairs[:40]:
            emit('  %3d | TGT %-44s DRF %s' % (i, t, d))
        if len(pairs) > 40:
            emit('  ... %d more' % (len(pairs) - 40))
    out.close()

    json.dump([{'fn': r[0], 'tgt': r[1], 'drf': r[2], 'byte': r[3], 'canon': r[4],
                'ndiff': None if r[6] is None else len(r[6]), 'class': r[5]} for r in rows],
              open(os.path.join(HERE, 'report.json'), 'w'), indent=1)


main()
