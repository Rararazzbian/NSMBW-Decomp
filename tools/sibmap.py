"""sibmap.py -- mechanical sibling-correspondence mapper.

Given a target .text range and a corpus of already-matching TUs, disassemble
everything with dtk (which zeroes relocated fields, so relocations mask
themselves), then for every target function find its closest precedent by
instruction-word correspondence.

Two similarity views per pair:
  * exact  -- raw instruction words (relocations already zeroed by dtk)
  * shape  -- immediates/displacements masked out, so "same code, different
              member offsets / different constants" still scores high

Usage:
    python sibmap.py disasm      # build the disassembly cache
    python sibmap.py map         # emit the correspondence report (JSON + text)
    python sibmap.py selftest    # negative control for the comparator
    python sibmap.py pair TGT CORPUS_TAG CORPUS_FN

Target selection (all commands that need a target):
    --lo 0x8002EF50 --hi 0x800311E0     address range, inclusive-exclusive
    --target a.txt,b.txt                disassembly files, comma separated or
                                        repeated; names are relative to the
                                        disassembly cache dir
    --dis DIR                           disassembly cache dir (else $SIBMAP_DIS)
    --out DIR                           where sibmap.json / sibmap.txt land
    --objs a.o,b.o                      disassemble these .o into the cache and
                                        use them as the target files
Equivalent environment variables: SIBMAP_LO, SIBMAP_HI, SIBMAP_TARGETS,
SIBMAP_DIS, SIBMAP_OUT.  With none of them given the module defaults below
apply, so old invocations keep working.
"""
import os
import re
import subprocess
import sys
import json
import difflib
from collections import defaultdict

ROOT = r'C:\Users\Razz\Documents\Projects\NSMBW-Decomp'
SCR = os.path.dirname(os.path.abspath(__file__))
# absolute so the copy in dfpakkun-shared/ reuses the cache built next door
DIS = os.environ.get('SIBMAP_DIS') or os.path.join(
    os.path.dirname(SCR) if os.path.basename(SCR) == 'dfpakkun-shared' else SCR, 'dis')
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')
OUT = os.environ.get('SIBMAP_OUT') or SCR

# Defaults kept so that a bare `python sibmap.py map` still reproduces the
# d_a_en_dfpakkun.cpp run; override with --lo/--hi/--target or the env vars.
TARGET_LO, TARGET_HI = 0x800281C0, 0x8002AB40
TARGET_FILES = ['a.txt', 'sinit.txt', 'b.txt']   # produced by hand, address order

FN_RE = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*(\w+)\s*$')
END_RE = re.compile(r'^\.endfn')
INSN_RE = re.compile(
    r'^/\*\s*([0-9A-F]{8})\s+[0-9A-F]{8}\s+((?:[0-9A-F]{2} ){3}[0-9A-F]{2})\s*\*/\s*(.*)$')

# ---------------------------------------------------------------- disassembly


def sh(*args):
    p = subprocess.run(list(args), cwd=ROOT, capture_output=True, text=True)
    return p.returncode


def build_cache():
    os.makedirs(DIS, exist_ok=True)
    jobs = []
    # corpus: every decompiled (== matching) TU's split object, straight out of
    # the original binary.
    for base in ('dol', 'lib', 'runtime'):
        top = os.path.join(ROOT, 'bin', 'dtkspl', 'obj', base)
        for dirpath, _, files in os.walk(top):
            for f in files:
                if f.endswith('.o'):
                    rel = os.path.relpath(os.path.join(dirpath, f), top)
                    tag = base + '_' + rel.replace(os.sep, '_')[:-2]
                    jobs.append((os.path.join(dirpath, f), tag))
    # REL split objects (all four RELs are fully matching)
    for rel in ('d_basesNP', 'd_en_bossNP', 'd_enemiesNP', 'd_profileNP'):
        top = os.path.join(ROOT, 'bin', 'dtkspl', rel, 'obj')
        if not os.path.isdir(top):
            continue
        for dirpath, _, files in os.walk(top):
            for f in files:
                if f.endswith('.o') and not f.startswith('auto_'):
                    rp = os.path.relpath(os.path.join(dirpath, f), top)
                    jobs.append((os.path.join(dirpath, f),
                                 'REL_' + rp.replace(os.sep, '_')[:-2]))
    # Our own compiled objects.  For a matching slice these are byte-exact
    # (verified in selftest against the split objects), and they additionally
    # carry the *weak copies* the linker discarded -- which is exactly what the
    # weak base-class members inside the target range are.
    top = os.path.join(ROOT, 'bin', 'compiled', 'wiimj2d')
    for dirpath, _, files in os.walk(top):
        for f in files:
            if f.endswith('.o'):
                rp = os.path.relpath(os.path.join(dirpath, f), top)
                jobs.append((os.path.join(dirpath, f),
                             'CMP_' + rp.replace(os.sep, '_')[:-2]))
    n = 0
    for src, tag in jobs:
        out = os.path.join(DIS, 'corpus_%s.txt' % tag)
        if os.path.exists(out):
            continue
        if not os.path.exists(src):
            print('  ! missing', src)
            continue
        if sh(DTK, 'elf', 'disasm', src, out) != 0:
            print('  ! disasm failed', src)
        n += 1
    print('disassembled %d objects into %s' % (n, DIS))


# -------------------------------------------------------------------- parsing


def parse(path):
    """-> list of dicts: name, addr, words, texts, relocs(bool per word)."""
    fns = []
    cur = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_RE.match(s)
            if m:
                cur = {'name': m.group(1), 'addr': None, 'words': [],
                       'texts': [], 'reloc': []}
                continue
            if END_RE.match(s):
                if cur and cur['words']:
                    fns.append(cur)
                cur = None
                continue
            if cur is None:
                continue
            m = INSN_RE.match(s)
            if m:
                if cur['addr'] is None:
                    cur['addr'] = int(m.group(1), 16)
                w = int(m.group(2).replace(' ', ''), 16)
                txt = m.group(3).split('/*')[0].strip()
                cur['words'].append(w)
                cur['texts'].append(txt)
                cur['reloc'].append(bool(RELOC_HINT.search(txt)))
    return [f for f in fns
            if not f['name'].startswith('gap_')
            and not f['name'].startswith('pad_')]


RELOC_HINT = re.compile(r'@(ha|l|sda21)\b|^bl\s+[A-Za-z_"]|^b\s+[A-Za-z_"]')

# ----------------------------------------------------------------- similarity

# Instruction "shape": zero out immediate / displacement fields so that a
# function differing only in member offsets or constants still lines up.
D_FORM = set(range(32, 56)) | {14, 15, 12, 13, 7, 8, 10, 11, 24, 25, 26, 27, 28, 29,
                               34, 36, 40, 44, 46, 47, 48, 50, 52, 54}


def shape(w):
    op = w >> 26
    if op == 18:                      # b / bl / ba / bla  -- mask LI
        return (op << 26) | (w & 3)
    if op == 16:                      # bc -- keep BO/BI, mask BD
        return w & 0xFFFF0003
    if op in (7, 8, 10, 11, 12, 13, 14, 15) or 32 <= op <= 55:
        return w & 0xFFFF0000         # D-form: mask the 16-bit field
    if op in (20, 21, 23):            # rlwimi / rlwinm / rlwnm -- keep the fields
        return w
    return w


def stats(a, b):
    """Return (n_equal_positional, len_a, len_b, aligned_matches)."""
    if len(a) == len(b):
        eq = sum(1 for x, y in zip(a, b) if x == y)
    else:
        eq = None
    sm = difflib.SequenceMatcher(a=a, b=b, autojunk=False)
    aligned = sum(bl.size for bl in sm.get_matching_blocks())
    return eq, len(a), len(b), aligned


def sim(a, b):
    _, la, lb, al = stats(a, b)
    return al / max(la, lb, 1)


def opsig(words):
    h = defaultdict(int)
    for w in words:
        h[w >> 26] += 1
    return h


def sigsim(h1, h2):
    keys = set(h1) | set(h2)
    inter = sum(min(h1.get(k, 0), h2.get(k, 0)) for k in keys)
    tot = max(sum(h1.values()), sum(h2.values()), 1)
    return inter / tot


# ---------------------------------------------------------------------- main


def load_target():
    fns = []
    for f in TARGET_FILES:
        fns += parse(os.path.join(DIS, f))
    fns = [f for f in fns if TARGET_LO <= f['addr'] < TARGET_HI]
    fns.sort(key=lambda f: f['addr'])
    return fns


def load_corpus(exclude_tags=()):
    out = []
    for f in sorted(os.listdir(DIS)):
        if not f.startswith('corpus_'):
            continue
        tag = f[len('corpus_'):-len('.txt')]
        if tag in exclude_tags:
            continue
        for fn in parse(os.path.join(DIS, f)):
            fn['tu'] = tag
            fn['shape'] = [shape(w) for w in fn['words']]
            fn['sig'] = opsig(fn['words'])
            out.append(fn)
    # dedupe: the same function reachable both as a dtk split object (ground
    # truth from the original binary) and as our own compiled object.  Prefer
    # the split copy; keep CMP_ entries only when they are unique, which is
    # exactly the weak copies the linker discarded.
    seen = {}
    for fn in out:
        key = (fn['name'], tuple(fn['words']))
        cur = seen.get(key)
        if cur is None or (cur['tu'].startswith('CMP_') and not fn['tu'].startswith('CMP_')):
            seen[key] = fn
    return list(seen.values())


# Enemy-actor TUs whose bodies are close enough to each other to be worth
# scoring as a family. KEEP THIS UP TO DATE: every newly landed enemy TU is a
# corpus member for the next one, and the list going stale has been reported
# twice as a reason a map missed precedents it should have found. The most
# recently banked units are the most valuable entries, not the least --
# d_a_en_bros_base alone contributed 99 matching functions.
FAMILY = ('dol_bases_d_a_en_dpakkun', 'dol_bases_d_a_en_dpakkun_base',
          'CMP_d_a_en_dpakkun', 'CMP_dol_bases_d_a_en_dpakkun',
          'dol_bases_d_a_en_lkuribo_base', 'dol_bases_d_a_en_kuribo_base',
          'dol_bases_d_a_en_net_nokonoko_base', 'dol_bases_d_a_en_super_bigpile',
          'dol_bases_d_a_enemy_ice', 'dol_bases_d_a_en_togezo_base',
          'dol_bases_d_a_en_shell', 'dol_bases_d_a_en_bigpile',
          'dol_bases_d_a_en_door', 'dol_bases_d_a_en_carry',
          'dol_bases_d_a_en_dfpakkun', 'dol_bases_d_a_en_jimen_pakkun_base',
          'dol_bases_d_a_en_bros_base', 'dol_bases_d_a_fireball_base',
          'dol_bases_d_a_en_eatcoin')

BASENAME = re.compile(r'^([A-Za-z_][A-Za-z0-9_]*(?:<[^>]*>)?)__')


def basename(n):
    m = BASENAME.match(n)
    return m.group(1) if m else n


def pair(tname, ctu, cname):
    """Side-by-side word diff of one target function against one precedent."""
    t = [f for f in load_target() if f['name'] == tname]
    if not t:
        sys.exit('target function %r not found' % tname)
    t = t[0]
    c = [f for f in parse(os.path.join(DIS, 'corpus_%s.txt' % ctu))
         if f['name'] == cname]
    if not c:
        sys.exit('corpus function %r not found in %s' % (cname, ctu))
    c = c[0]
    sm = difflib.SequenceMatcher(a=t['words'], b=c['words'], autojunk=False)
    print('%s (%d) vs %s::%s (%d)' % (tname, len(t['words']), ctu, cname, len(c['words'])))
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            print('    == %d words' % (i2 - i1))
            continue
        n = max(i2 - i1, j2 - j1)
        for k in range(n):
            a = ('%08X %-42s' % (t['words'][i1 + k], t['texts'][i1 + k])) if i1 + k < i2 else ' ' * 51
            b = ('%08X %s' % (c['words'][j1 + k], c['texts'][j1 + k])) if j1 + k < j2 else ''
            print('  %-3s %s | %s' % (tag[:3], a, b))


def selftest():
    """Negative control: the comparator must NOT call different things equal."""
    tgt = load_target()
    assert tgt, 'no target functions parsed'
    # positive control: every function is 1.000 against itself
    for f in tgt:
        assert sim(f['words'], f['words']) == 1.0, 'self-similarity broken on ' + f['name']
    # negative control: the largest and the smallest function must not be equal.
    # (Do NOT use tgt[0] vs tgt[1] -- adjacent thunks are often genuinely
    # word-identical, which makes that check fire on a true positive.)
    big = max(tgt, key=lambda f: len(f['words']))
    small = min(tgt, key=lambda f: len(f['words']))
    s = sim(big['words'], small['words'])
    assert s < 1.0, 'NEGATIVE CONTROL FAILED: two different functions equal'
    # mutation control: flipping one word must drop the score below 1.0
    mut = list(big['words'])
    mut[len(mut) // 2] ^= 0x00010000
    assert sim(big['words'], mut) < 1.0, 'MUTATION CONTROL FAILED'
    print('positive control ok: sim(f,f)=1.000 for all %d target functions' % len(tgt))
    print('negative control ok: sim(%s,%s)=%.3f' % (big['name'][:24], small['name'][:24], s))
    print('mutation control ok: one flipped word -> %.4f'
          % sim(big['words'], mut))
    # word-identical clusters inside the target itself (reported, not asserted)
    from collections import defaultdict as _dd
    cl = _dd(list)
    for f in tgt:
        cl[tuple(f['words'])].append(f['name'])
    dup = [v for v in cl.values() if len(v) > 1]
    print('word-identical in-file clusters: %d' % len(dup))
    # parity between a dtk split object and our own compiled object
    sp = {f['name']: f['words'] for f in parse(os.path.join(DIS, 'corpus_dol_bases_d_a_en_dpakkun.txt'))}
    cm = {f['name']: f['words'] for f in parse(os.path.join(DIS, 'corpus_CMP_dol_bases_d_a_en_dpakkun.txt'))}
    common = set(sp) & set(cm)
    bad = [n for n in common if sp[n] != cm[n]]
    print('split-vs-compiled parity on d_a_en_dpakkun: %d/%d functions identical, %d differ'
          % (len(common) - len(bad), len(common), len(bad)))
    for n in bad[:10]:
        print('   differ:', n, len(sp[n]), len(cm[n]),
              sum(1 for x, y in zip(sp[n], cm[n]) if x != y) if len(sp[n]) == len(cm[n]) else '-')
    print('names only in split:', sorted(set(sp) - set(cm))[:10])
    print('names only in compiled:', sorted(set(cm) - set(sp))[:10])


def report():
    tgt = load_target()
    corpus = load_corpus(exclude_tags=())
    print('target functions: %d   corpus functions: %d' % (len(tgt), len(corpus)))

    for f in tgt:
        f['shape'] = [shape(w) for w in f['words']]
        f['sig'] = opsig(f['words'])

    # intra-file clusters
    by_words = defaultdict(list)
    for f in tgt:
        by_words[tuple(f['words'])].append(f['name'])
    by_shape = defaultdict(list)
    for f in tgt:
        by_shape[tuple(f['shape'])].append(f['name'])

    results = []
    for f in tgt:
        n = len(f['words'])
        cands = []
        for c in corpus:
            m = len(c['words'])
            if abs(m - n) > max(6, 0.5 * n):
                continue
            if sigsim(f['sig'], c['sig']) < 0.55:
                continue
            cands.append(c)
        scored = []
        for c in cands:
            se = sim(f['words'], c['words'])
            ss = sim(f['shape'], c['shape'])
            scored.append((se, ss, c))
        scored.sort(key=lambda t: (-(t[0] * 2 + t[1]), t[2]['tu']))
        top = []
        for se, ss, c in scored[:6]:
            eq, la, lb, al = stats(f['words'], c['words'])
            eqs, _, _, als = stats(f['shape'], c['shape'])
            top.append({
                'tu': c['tu'], 'name': c['name'], 'addr': c['addr'],
                'len': lb, 'exact_sim': round(se, 4), 'shape_sim': round(ss, 4),
                'pos_equal_words': eq, 'aligned_words': al,
                'shape_pos_equal': eqs, 'shape_aligned': als,
                'word_diffs': (None if eq is None else lb - eq),
            })
        # same-basename hits anywhere in the corpus, and best hit restricted to
        # the pakkun / recent-actor family, regardless of global rank.
        bn = basename(f['name'])
        namehits = []
        for c in corpus:
            if basename(c['name']) != bn:
                continue
            se, ss = sim(f['words'], c['words']), sim(f['shape'], c['shape'])
            eq, la, lb, al = stats(f['words'], c['words'])
            namehits.append({'tu': c['tu'], 'name': c['name'], 'len': lb,
                             'exact_sim': round(se, 4), 'shape_sim': round(ss, 4),
                             'pos_equal_words': eq, 'aligned_words': al})
        namehits.sort(key=lambda h: -(h['exact_sim'] * 2 + h['shape_sim']))
        famhits = []
        for c in corpus:
            if c['tu'] not in FAMILY:
                continue
            if abs(len(c['words']) - n) > max(8, 0.8 * n):
                continue
            se, ss = sim(f['words'], c['words']), sim(f['shape'], c['shape'])
            if se < 0.35 and ss < 0.45:
                continue
            eq, la, lb, al = stats(f['words'], c['words'])
            famhits.append({'tu': c['tu'], 'name': c['name'], 'len': lb,
                            'exact_sim': round(se, 4), 'shape_sim': round(ss, 4),
                            'pos_equal_words': eq, 'aligned_words': al})
        famhits.sort(key=lambda h: -(h['exact_sim'] * 2 + h['shape_sim']))
        results.append({
            'name': f['name'], 'addr': f['addr'], 'insns': n, 'bytes': n * 4,
            'top': top, 'namehits': namehits[:5], 'famhits': famhits[:5],
            'twins_exact': [x for x in by_words[tuple(f['words'])] if x != f['name']],
            'twins_shape': [x for x in by_shape[tuple(f['shape'])] if x != f['name']],
        })

    with open(os.path.join(OUT, 'sibmap.json'), 'w') as fh:
        json.dump(results, fh, indent=1)
    print('wrote sibmap.json')

    # human-readable dump
    with open(os.path.join(OUT, 'sibmap.txt'), 'w', encoding='utf-8') as fh:
        for r in results:
            fh.write('\n%08X  %-70s %4d insns / %5d B\n'
                     % (r['addr'], r['name'], r['insns'], r['bytes']))
            if r['twins_exact']:
                fh.write('   BIT-IDENTICAL IN-FILE TWINS: %s\n' % ', '.join(r['twins_exact']))
            elif r['twins_shape']:
                fh.write('   same-shape in-file twins: %s\n' % ', '.join(r['twins_shape']))
            for label, key in (('GLOBAL', 'top'), ('NAME  ', 'namehits'), ('FAMILY', 'famhits')):
                for t in r[key]:
                    fh.write('  %s %-42s %-52s len=%-4d exact=%.3f shape=%.3f aligned=%d\n'
                             % (label, t['tu'][:42], t['name'][:52], t['len'],
                                t['exact_sim'], t['shape_sim'], t['aligned_words']))
    print('wrote sibmap.txt')


def apply_options(argv):
    """Consume --lo/--hi/--target/--dis/--out/--objs; return the rest.

    Options may also arrive as environment variables, and anything left unset
    keeps the module-level default, so pre-argv invocations are unchanged.
    """
    global TARGET_LO, TARGET_HI, TARGET_FILES, DIS, OUT
    rest = []
    opts = {}
    i = 0
    while i < len(argv):
        a = argv[i]
        if a.startswith('--'):
            key = a[2:]
            if '=' in key:
                key, val = key.split('=', 1)
            else:
                i += 1
                if i >= len(argv):
                    sys.exit('option --%s needs a value' % key)
                val = argv[i]
            if key not in ('lo', 'hi', 'target', 'dis', 'out', 'objs'):
                sys.exit('unknown option --%s' % key)
            if key in ('target', 'objs'):
                opts.setdefault(key, []).extend(
                    v for v in val.replace(';', ',').split(',') if v)
            else:
                opts[key] = val
        else:
            rest.append(a)
        i += 1

    env_lo, env_hi = os.environ.get('SIBMAP_LO'), os.environ.get('SIBMAP_HI')
    env_tg = os.environ.get('SIBMAP_TARGETS')
    if 'dis' in opts:
        DIS = os.path.abspath(opts['dis'])
    OUT = os.path.abspath(opts['out']) if 'out' in opts else (
        os.environ.get('SIBMAP_OUT') or OUT)
    lo = opts.get('lo', env_lo)
    hi = opts.get('hi', env_hi)
    if lo is not None:
        TARGET_LO = int(lo, 0)
    if hi is not None:
        TARGET_HI = int(hi, 0)
    if TARGET_HI <= TARGET_LO:
        sys.exit('empty target range %08X-%08X' % (TARGET_LO, TARGET_HI))

    files = opts.get('target')
    if not files and env_tg:
        files = [v for v in env_tg.replace(';', ',').split(',') if v]
    if 'objs' in opts:
        os.makedirs(DIS, exist_ok=True)
        produced = []
        for o in opts['objs']:
            src = o if os.path.isabs(o) else os.path.join(ROOT, o)
            if not os.path.exists(src):
                sys.exit('no such object: %s' % src)
            name = os.path.splitext(os.path.basename(src))[0] + '.txt'
            out = os.path.join(DIS, name)
            if not os.path.exists(out) and sh(DTK, 'elf', 'disasm', src, out) != 0:
                sys.exit('disasm failed: %s' % src)
            produced.append(name)
        files = (files or []) + produced
    if files:
        TARGET_FILES = files
    return rest


def main(argv):
    argv = apply_options(argv)
    cmd = argv[0] if argv else 'map'
    if cmd == 'disasm':
        build_cache()
    elif cmd == 'selftest':
        selftest()
    elif cmd == 'pair':
        if len(argv) < 4:
            sys.exit('usage: sibmap.py pair TARGET_FN CORPUS_TAG CORPUS_FN')
        pair(argv[1], argv[2], argv[3])
    elif cmd in ('map', 'report'):
        report()
    else:
        sys.exit('unknown command %r (disasm|map|selftest|pair)' % cmd)


if __name__ == '__main__':
    main(sys.argv[1:])
