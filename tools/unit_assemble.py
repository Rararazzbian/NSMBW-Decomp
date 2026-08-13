"""Assemble the six demo_manager batch drafts into one TU, in TARGET ADDRESS ORDER.

Source order controls emission order for functions AND for their literal pools,
and there is no batch-level ordering to preserve -- only the target's address
order. So every definition is placed individually, by address, not by batch.

Emits `source/dol/bases/d_a_player_demo_manager.cpp` and prints the address-order
plan plus any function it could not place, because an unplaced definition is a
finding, not noise.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))

BATCHES = ['dm-b1', 'dm-b2', 'dm-b3', 'dm-b4', 'dm-b5', 'dm-b6']
OUT = os.path.join(ROOT, 'source', 'dol', 'bases', 'd_a_player_demo_manager.cpp')
SYMS = os.path.join(ROOT, 'bin', 'dtk', 'wiimj2d_symbols.txt')
LO, HI = 0x8005B3A0, 0x8005D7E0

# Which source spelling maps to which mangled symbol. Members are matched by
# `daPyDemoMng_c::<name>`; the two file-statics and the free function by name.
SYM_RE = re.compile(
    r'(\S+)\s*=\s*\.text:0x([0-9A-Fa-f]{8}).*?type:function size:0x([0-9A-Fa-f]+)')


def target_order():
    """-> [(addr, size, mangled)] in address order, for our range."""
    out = []
    for line in open(SYMS, encoding='utf-8'):
        m = SYM_RE.search(line)
        if m:
            a = int(m.group(2), 16)
            if LO <= a < HI:
                out.append((a, int(m.group(3), 16), m.group(1)))
    out.sort()
    return out


def demangle_key(mangled):
    """A loose key we can look for in source: the bare function name."""
    if mangled.startswith('__ct__'):
        return 'daPyDemoMng_c::daPyDemoMng_c'
    if mangled.startswith('__dt__'):
        return 'daPyDemoMng_c::~daPyDemoMng_c'
    if mangled.startswith('fn_'):
        return mangled
    if mangled.startswith('__sinit') or mangled.startswith('__arraydtor'):
        return None                      # compiler-generated, never hand-written
    name = mangled.split('__')[0]
    return 'daPyDemoMng_c::' + name


# A definition's signature line. Deliberately NOT anchored with a consuming
# `^[A-Za-z_]`: that ate the first character and killed the \b before
# `daPyDemoMng_c::`, so every constructor and destructor silently went unmatched.
NAME_RE = r'(daPyDemoMng_c::~?\w+|fn_[0-9A-Fa-f]{8}|makeCourseInList)'
DEF_SIG = re.compile(r'^(?=\S)(?!//|#).*' + NAME_RE + r'\s*\(')

# Invented source names for functions the symbol map leaves unnamed. Keeping
# this mapping explicit is the whole point -- discarding invented names as
# "unmatched extras" desynchronised a positional comparison once before.
ALIASES = {'fn_8005D280': 'makeCourseInList'}


def split_file(path):
    """-> (includes, preamble_chunks, [(key, text)]) for one batch draft."""
    lines = open(path, encoding='utf-8').read().splitlines(True)
    includes, preamble, defs = [], [], []
    i = 0
    pending = []                          # comments/blank lines not yet attached
    while i < len(lines):
        l = lines[i]
        if l.startswith('#include'):
            includes.append(l)
            pending = []
            i += 1
            continue
        m = DEF_SIG.match(l.rstrip()) if l.strip() else None
        # a prototype ends in ';' on the signature line -- not a definition
        if m and not l.rstrip().endswith(';'):
            # the opening brace may be on this line or a following one
            k = i
            while k < len(lines) and '{' not in lines[k]:
                k += 1
            if k >= len(lines):
                preamble.append(''.join(pending) + l)
                pending = []
                i += 1
                continue
            depth = 0
            j = k
            while j < len(lines):
                depth += lines[j].count('{') - lines[j].count('}')
                j += 1
                if depth == 0:
                    break
            body = ''.join(pending) + ''.join(lines[i:j])
            defs.append((m.group(1), body))
            pending = []
            i = j
            continue
        if l.strip() == '' or l.lstrip().startswith('//'):
            pending.append(l)
            i += 1
            continue
        # anything else at file scope: static inline helpers, extern decls, etc.
        preamble.append(''.join(pending) + l)
        pending = []
        i += 1
    return includes, preamble, defs


def main():
    all_inc, all_pre, by_key = [], [], {}
    for b in BATCHES:
        inc, pre, defs = split_file(os.path.join(HERE, b + '.cpp'))
        for x in inc:
            if x not in all_inc:
                all_inc.append(x)
        # Dedupe whole preamble BLOCKS, never individual lines: a per-line
        # dedupe drops every repeated `}` after the first and silently truncates
        # the file-scope inline helpers. That produced a wall of "declaration
        # syntax error" at unrelated functions, which reads like a merge bug
        # anywhere except where it actually was.
        blob = ''.join(pre)
        if blob.strip() and blob not in all_pre:
            all_pre.append(blob)
        for key, body in defs:
            if key:
                by_key.setdefault(key, []).append((b, body))

    order = target_order()

    # Overloads share a source key (both isDemoMode spellings map to
    # `daPyDemoMng_c::isDemoMode`), so a plain dict emitted one body TWICE and
    # dropped the other. Pair them explicitly: source definitions sorted by
    # parameter count against target symbols sorted by address, and print the
    # pairing so a wrong guess is visible rather than silent.
    def nparams(text):
        sig = text[text.index('('):text.index(')')] if '(' in text else ''
        return 0 if not sig.strip('( ') else sig.count(',') + 1

    tgt_by_key = {}
    for a, size, mangled in order:
        k = demangle_key(mangled)
        if k:
            tgt_by_key.setdefault(k, []).append(a)

    assignment = {}
    for k, addrs in tgt_by_key.items():
        cands = by_key.get(k) or by_key.get(ALIASES.get(k, '')) or []
        if len(addrs) > 1 or len(cands) > 1:
            cands = sorted(cands, key=lambda c: nparams(c[1]))
            print('overload set %s: %d target addresses, %d source definitions'
                  % (k, len(addrs), len(cands)))
            for a, c in zip(sorted(addrs), cands):
                print('    0x%08X  <- %d-param definition from %s'
                      % (a, nparams(c[1]), c[0]))
        for a, c in zip(sorted(addrs), cands):
            assignment[a] = c

    placed, missing, used = [], [], set()
    for a, size, mangled in order:
        key = demangle_key(mangled)
        if key is None:
            placed.append((a, mangled, None, None))
            continue
        hit = assignment.get(a)
        if hit is None:
            missing.append((a, mangled, key))
            placed.append((a, mangled, None, None))
        else:
            placed.append((a, mangled, hit[0], hit[1]))
            used.add(id(hit))

    print('%-12s %-46s %s' % ('addr', 'symbol', 'from'))
    for a, mangled, batch, _ in placed:
        print('0x%08X   %-46s %s' % (a, mangled[:46], batch or '(compiler-generated)'))

    if missing:
        print('\n!! %d target functions had NO source definition:' % len(missing))
        for a, mangled, key in missing:
            print('   0x%08X  %s   (looked for %s)' % (a, mangled, key))

    unused = [k for k, v in by_key.items()
              if not any(id(c) in used for c in v)]
    if unused:
        print('\n!! %d source definitions were NOT placed (an unplaced definition is a\n'
              '   finding, not noise -- it means a name mismatch or an extra function):'
              % len(unused))
        for k in unused:
            print('   %s  (from %s)' % (k, by_key[k][0][0]))

    body = []
    body.append('#include <game/bases/d_a_player_demo_manager.hpp>\n')
    for x in all_inc:
        if 'd_a_player_demo_manager.hpp' not in x:
            body.append(x)
    body.append('\n')
    for x in all_pre:
        body.append(x if x.endswith('\n') else x + '\n')
    body.append('\n')
    # The TU owns .sbss 0xd0-0xd8: c_StartPointKinokoHouseID (a `static int` in
    # d_wm_lib.hpp, emitted into whichever TU odr-uses it) plus this singleton
    # pointer. Defining it here is what allows the syms.txt entry to be deleted;
    # the two must move together or the link sees either no definition or two.
    body.append('daPyDemoMng_c *daPyDemoMng_c::mspInstance;\n\n')
    for a, mangled, batch, text in placed:
        if text:
            body.append(text if text.endswith('\n') else text + '\n')
            body.append('\n')

    with open(OUT, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(''.join(body))
    print('\nwrote %s (%d definitions placed, %d missing)'
          % (OUT, len(used), len(missing)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
