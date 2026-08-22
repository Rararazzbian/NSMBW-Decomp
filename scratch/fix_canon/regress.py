"""Regression harness for the harness.canonicalise() fix.

Goal (per task spec):
  (a) every function that is byte-identical between a landed draft object and
      the corresponding original (target) object must STILL compare equal
      under canonicalise() -- no new false negatives.
  (b) genuinely different functions must STILL compare unequal under
      canonicalise() -- the fix must not have made the comparator too
      permissive.

Corpus: every object under bin/compiled/<module>/... (our own recompiled,
landed, byte-exact sources) paired by FUNCTION NAME against every object
under bin/dtkspl/<module>/obj (wiimj2d) or bin/dtkspl/obj (the DOL), which is
dtk's own split of the ORIGINAL binary into objects. Name-based pairing sidesteps
address-range bookkeeping entirely: a function's disassembly is looked up by
its (normalised) name in a merged target dictionary built from ALL split
objects of the module, so no per-file address mapping is needed.

Disassembly is cached under scratch/fix_canon/disasm_cache/ so repeat runs are
fast.
"""
import glob
import json
import os
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'auto_decomp'))
import harness  # noqa: E402

CACHE = os.path.join(ROOT, 'scratch', 'fix_canon', 'disasm_cache')
os.makedirs(CACHE, exist_ok=True)

MODULES = {
    'wiimj2d':   {'draft': 'bin/compiled/wiimj2d',   'target': 'bin/dtkspl/obj'},
    'd_basesNP': {'draft': 'bin/compiled/d_basesNP', 'target': 'bin/dtkspl/d_basesNP/obj'},
    'd_enemiesNP': {'draft': 'bin/compiled/d_enemiesNP', 'target': 'bin/dtkspl/d_enemiesNP/obj'},
    'd_en_bossNP': {'draft': 'bin/compiled/d_en_bossNP', 'target': 'bin/dtkspl/d_en_bossNP/obj'},
    'd_profileNP': {'draft': 'bin/compiled/d_profileNP', 'target': 'bin/dtkspl/d_profileNP/obj'},
}


def cached_disasm(obj_path):
    """Disassemble obj_path via dtk, caching the .txt by a stable key."""
    key = obj_path.replace(os.sep, '_').replace('/', '_').replace(':', '')
    out = os.path.join(CACHE, key + '.txt')
    if not os.path.exists(out) or os.path.getmtime(out) < os.path.getmtime(obj_path):
        ok, log = harness.disasm(obj_path, out)
        if not ok:
            sys.stderr.write('disasm FAILED for %s: %s\n' % (obj_path, log[:300]))
            return None
    return out


# ---------------------------------------------------------------- raw bytes

import re
FN_START = harness.FN_START
FN_END = harness.FN_END
WORD_RE = re.compile(r'^/\*\s*\S+\s+\S+\s+((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})\s*\*/')


def all_function_bodies(disasm_path, want_bytes=False):
    """name(normalised) -> canonicalised-text body (or raw-byte-word list).

    First occurrence wins per file, mirroring harness.extract()'s own
    first-match convention. Skips gap_* padding functions implicitly (FN_START
    is only emitted by harness.list_functions handling, but here we just parse
    raw .fn/.endfn pairs directly).
    """
    out = {}
    if not disasm_path or not os.path.exists(disasm_path):
        return out
    name = None
    words = []
    lines = []
    with open(disasm_path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                name = harness.norm_name(m.group(1))
                words, lines = [], []
                continue
            if FN_END.match(s):
                if name is not None and name not in out:
                    if want_bytes:
                        out[name] = words
                    else:
                        out[name] = harness.canonicalise(lines) if lines else []
                name = None
                continue
            if name is not None:
                if want_bytes:
                    mw = WORD_RE.match(s)
                    if mw:
                        words.append(mw.group(1).replace(' ', ''))
                else:
                    mi = harness.INSN.match(s)
                    if mi:
                        lines.append(mi.group(1).strip())
    return out


def only_auto_split(paths):
    """Keep only the genuine original-binary address split (auto_*.o).

    bin/dtkspl/<module>/obj also contains a SHADOW tree mirroring source/ paths
    (e.g. obj/dol/bases/d_a_en_bigpile.o) -- the build's own recompiled copy of
    already-landed sources, placed there so the final .rel/.dol can be
    reconstructed by mixing landed-fresh-compiles with everything else copied
    verbatim from the real original. Those shadow objects are NOT the retail
    original; mixing them into the "target" side pairs a draft against another
    draft and produces bogus comparator verdicts in both directions. Only
    `auto_*` files are the real dtk split-by-address objects (see
    AGENT_CONTEXT.md: "bin/dtkspl/obj/auto_*_*.o -- the original binary split
    into objects by address").
    """
    return [p for p in paths if os.path.basename(p).startswith('auto_')]


def build_merged_dict(paths, want_bytes=False):
    merged = {}
    for p in paths:
        d = cached_disasm(p)
        fns = all_function_bodies(d, want_bytes=want_bytes)
        for k, v in fns.items():
            merged.setdefault(k, v)  # first object wins, like extract()
    return merged


def main():
    t0 = time.time()
    report = {}

    total_byte_eq = 0
    total_canon_eq_given_byte_eq = 0
    regressions = []          # byte-equal but canonicalise says unequal
    byte_neq_canon_eq = []    # byte-different but canonicalise says equal -- the dangerous direction
    per_module = {}

    for module, paths in MODULES.items():
        draft_dir = os.path.join(ROOT, paths['draft'])
        target_dir = os.path.join(ROOT, paths['target'])
        if not os.path.isdir(draft_dir) or not os.path.isdir(target_dir):
            continue
        draft_objs = glob.glob(os.path.join(draft_dir, '**', '*.o'), recursive=True)
        target_objs = only_auto_split(glob.glob(os.path.join(target_dir, '**', '*.o'), recursive=True))
        if not draft_objs or not target_objs:
            continue

        sys.stderr.write('[%s] %d draft objs, %d target objs -- disassembling...\n'
                          % (module, len(draft_objs), len(target_objs)))

        target_bytes = build_merged_dict(target_objs, want_bytes=True)
        target_text = build_merged_dict(target_objs, want_bytes=False)
        draft_bytes = build_merged_dict(draft_objs, want_bytes=True)
        draft_text = build_merged_dict(draft_objs, want_bytes=False)

        common = sorted(set(target_bytes) & set(draft_bytes))
        m_byte_eq = 0
        m_canon_eq_given_byte_eq = 0
        m_regressions = []
        m_byte_neq_canon_eq = []

        for name in common:
            tb, db = target_bytes[name], draft_bytes[name]
            tt, dt = target_text.get(name), draft_text.get(name)
            byte_eq = (tb == db) and len(tb) > 0
            canon_eq = (tt == dt) if (tt is not None and dt is not None) else False
            if byte_eq:
                m_byte_eq += 1
                if canon_eq:
                    m_canon_eq_given_byte_eq += 1
                else:
                    m_regressions.append(name)
            else:
                if canon_eq and len(tb) > 0 and len(db) > 0:
                    m_byte_neq_canon_eq.append(name)

        total_byte_eq += m_byte_eq
        total_canon_eq_given_byte_eq += m_canon_eq_given_byte_eq
        regressions.extend('%s:%s' % (module, n) for n in m_regressions)
        byte_neq_canon_eq.extend('%s:%s' % (module, n) for n in m_byte_neq_canon_eq)

        per_module[module] = {
            'common_functions': len(common),
            'byte_equal': m_byte_eq,
            'canon_equal_given_byte_equal': m_canon_eq_given_byte_eq,
            'regressions (byte-eq but canon says NOT eq)': m_regressions,
            'DANGEROUS (byte-diff but canon says eq)': m_byte_neq_canon_eq,
        }
        sys.stderr.write('[%s] common=%d byte_eq=%d canon_eq_given_byte_eq=%d '
                          'regressions=%d dangerous=%d\n'
                          % (module, len(common), m_byte_eq, m_canon_eq_given_byte_eq,
                             len(m_regressions), len(m_byte_neq_canon_eq)))

    # ------------------------------------------------------- cross-pair negative test
    # Guard against "canonicalise degenerated to always-true": pair each target
    # function's body against a DIFFERENT function's draft body (offset by one in
    # the sorted name list, within the same module) and confirm they compare
    # UNEQUAL -- UNLESS the two are genuinely byte-identical machine code (many
    # trivial one-line stubs and template instantiations across different classes
    # really do compile to the same bytes; AGENT_CONTEXT.md: "Two functions with
    # the same body are indistinguishable in .text." That is not a canonicalise
    # defect, so this test only flags a pair as a false positive when the RAW
    # BYTES differ but canonicalise still calls them equal -- the actually
    # dangerous case, and the same test the 10dff97 fix commit ran by hand.
    cross_pair_checked = 0
    cross_pair_skipped_genuinely_identical = 0
    cross_pair_false_positive = []
    for module, paths in MODULES.items():
        draft_dir = os.path.join(ROOT, paths['draft'])
        target_dir = os.path.join(ROOT, paths['target'])
        if not os.path.isdir(draft_dir) or not os.path.isdir(target_dir):
            continue
        draft_objs = glob.glob(os.path.join(draft_dir, '**', '*.o'), recursive=True)
        target_objs = only_auto_split(glob.glob(os.path.join(target_dir, '**', '*.o'), recursive=True))
        target_text = build_merged_dict(target_objs, want_bytes=False)
        draft_text = build_merged_dict(draft_objs, want_bytes=False)
        target_byt = build_merged_dict(target_objs, want_bytes=True)
        draft_byt = build_merged_dict(draft_objs, want_bytes=True)
        names = sorted(set(target_text) & set(draft_text))
        if len(names) < 2:
            continue
        for i, name in enumerate(names):
            other = names[(i + 1) % len(names)]
            if other == name:
                continue
            a = target_text[name]
            b = draft_text[other]
            if not a or not b:
                continue
            cross_pair_checked += 1
            if a == b:
                ab = target_byt.get(name)
                bb = draft_byt.get(other)
                if ab is not None and bb is not None and ab == bb:
                    # Genuinely identical machine code under two different
                    # names -- correct, not a comparator defect.
                    cross_pair_skipped_genuinely_identical += 1
                else:
                    cross_pair_false_positive.append('%s: target[%s] == draft[%s]'
                                                      % (module, name, other))

    # ------------------------------------------------------- synthetic unit tests
    # Minimal line-pairs reproducing exactly the categories the original fix
    # commit (10dff97) claimed to have checked: different callees, different
    # pool symbols, different registers, different offsets, adjacent section
    # indices. Each pair MUST canonicalise UNEQUAL.
    synthetic_cases = [
        ('different callee (fn placeholder)',
         ['bl fn_800A1234'], ['bl fn_800CDEF0']),
        ('different callee (mangled placeholder, same address form)',
         ['bl fn_800C3B20'], ['bl fn_800C3C00__FP10dLineMng_c']),
        ('different pool symbol value (0.0f-style vs 8.0f-style, two distinct refs)',
         ['lfs f0, "@100_80000000"@sda21(r0)', 'lfs f1, "@200_80000010"@sda21(r0)'],
         ['lfs f0, "@11"@sda21(r0)', 'lfs f1, "@11"@sda21(r0)']),
        ('different registers',
         ['lis r31, "@49614_80359100"@ha'], ['lis r29, "@49614_80359100"@ha']),
        ('different immediate offset',
         ['addi r4, r31, 0x250'], ['addi r4, r31, 0x2d0']),
        ('adjacent section indices, TWO distinct refs each side (...bss.0/.1 vs both .0)',
         ['lis r29, ...bss.0@ha', 'lis r28, ...bss.1@ha'],
         ['lis r29, ...bss.0@ha', 'lis r28, ...bss.0@ha']),
        ('adjacent section indices, TWO distinct refs each side (...sbss.1/.12 vs both .1)',
         ['lwz r3, ...sbss.1@sda21(r13)', 'lwz r4, ...sbss.12@sda21(r13)'],
         ['lwz r3, ...sbss.1@sda21(r13)', 'lwz r4, ...sbss.1@sda21(r13)']),
        ('quoted vtable/template name, genuinely different template arg',
         ['lis r30, "__vt__25sFStateID_c<10dLineMng_c>"@ha'],
         ['lis r30, "__vt__24sFStateID_c<9dSomeOther_c>"@ha']),
    ]
    synthetic_results = []
    synthetic_failures = []
    for label, a, b in synthetic_cases:
        ca, cb = harness.canonicalise(a), harness.canonicalise(b)
        eq = (ca == cb)
        synthetic_results.append((label, a, b, ca, cb, eq))
        if eq:
            synthetic_failures.append(label)

    # Regex-level check for the lazy-quantifier defect 10dff97 also fixed:
    # `...sbss.12` must be consumed WHOLE, not split into `...sbss.1` + a
    # stray literal `2` that would then survive substitution and corrupt the
    # line. This is a direct check on POOL_SYM, not on canonicalise()'s
    # per-side renumbering (which -- correctly, see the docstring -- cannot
    # distinguish "the only reference" regardless of its numeric suffix).
    lazy_quantifier_cases = [
        ('...sbss.12@sda21(r13)', '...sbss.12'),
        ('...bss.123@ha', '...bss.123'),
        ('...data.7@ha', '...data.7'),
        ('...rodata.45@l', '...rodata.45'),
        ('...sdata2.9@sda21(r0)', '...sdata2.9'),
    ]
    lazy_quantifier_failures = []
    for line, expect in lazy_quantifier_cases:
        m = harness.POOL_SYM.search(line)
        got = m.group(0) if m else None
        if got != expect:
            lazy_quantifier_failures.append((line, expect, got))

    # ------------------------------------------------------- positive unit tests
    # These MUST compare equal -- same-shape pool refs / quoted forms / CFront
    # placeholder-callee mangling, differing only in naming, not semantics.
    positive_cases = [
        ('quoted pool sym vs unresolved bss form (the reported bug)',
         ['lis r31, "@49614_80359100"@ha', 'addi r31, r31, "@49614_80359100"@l'],
         ['lis r31, ...bss.0@ha', 'addi r31, r31, ...bss.0@l']),
        ('placeholder callee, CFront-mangled vs bare',
         ['bl fn_800C3B20'], ['bl fn_800C3B20__FP10dLineMng_c']),
        ('same quoted template name on both sides',
         ['lis r30, "__vt__25sFStateID_c<10dLineMng_c>"@ha'],
         ['lis r30, "__vt__25sFStateID_c<10dLineMng_c>"@ha']),
        ('sdata2 two-part target name vs bare draft pool id',
         ['lfs f0, "@71831_8042B7EC"@sda21(r0)'], ['lfs f0, "@21389"@sda21(r0)']),
    ]
    positive_failures = []
    for label, a, b in positive_cases:
        ca, cb = harness.canonicalise(a), harness.canonicalise(b)
        if ca != cb:
            positive_failures.append((label, a, b, ca, cb))

    # ------------------------------------------------------- the specific case
    target_ll = harness.extract(
        os.path.join(ROOT, 'wip', 'agent_line_mng', 'work', 'target.txt'),
        'executeState_Left30Left__10dLineMng_cFv')
    draft_ll = harness.extract(
        os.path.join(ROOT, 'wip', 'fix_bigtwo', '_tally', 'd.txt'),
        'executeState_Left30Left__10dLineMng_cFv')
    left30left_equal = (target_ll is not None and draft_ll is not None and target_ll == draft_ll)

    elapsed = time.time() - t0

    result = {
        'elapsed_sec': round(elapsed, 1),
        'per_module': per_module,
        'TOTAL byte_equal functions checked': total_byte_eq,
        'TOTAL also canon_equal (should == byte_equal count)': total_canon_eq_given_byte_eq,
        'TOTAL regressions (byte-eq, canon says NOT eq -- should be 0)': len(regressions),
        'regression list': regressions,
        'TOTAL dangerous (byte-diff, canon says eq -- should be 0)': len(byte_neq_canon_eq),
        'dangerous list': byte_neq_canon_eq,
        'cross_pair_checked (different-function pairs)': cross_pair_checked,
        'cross_pair_skipped_genuinely_identical_bytes': cross_pair_skipped_genuinely_identical,
        'cross_pair_false_positive (byte-diff, canon says eq -- should be 0)': cross_pair_false_positive,
        'synthetic_negative_cases_checked': len(synthetic_cases),
        'synthetic_negative_failures (should be empty)': synthetic_failures,
        'lazy_quantifier_cases_checked': len(lazy_quantifier_cases),
        'lazy_quantifier_failures (should be empty)': lazy_quantifier_failures,
        'positive_cases_checked': len(positive_cases),
        'positive_failures (should be empty)': positive_failures,
        'executeState_Left30Left now canonical-EQUAL': left30left_equal,
    }
    print(json.dumps(result, indent=2))

    with open(os.path.join(os.path.dirname(__file__), 'regress_result.json'), 'w', encoding='utf-8') as fh:
        json.dump(result, fh, indent=2)


if __name__ == '__main__':
    main()
