"""Automated match loop: propose C++, compile, diff, iterate until byte-exact.

The whole reason this works unattended is that success is OBJECTIVE and CHEAP.
A function either assembles to the same bytes as the original or it does not,
and finding out costs one compile (~1s). No human judgement in the loop, so a
model can grind without supervision.

    python tools/auto_decomp/harness.py --unit dol/bases/d_a_foo.cpp --list
    python tools/auto_decomp/harness.py --unit dol/bases/d_a_foo.cpp --fn create__6dFoo_cFv

Design notes, learned the hard way this session:

* NOTHING here writes to source/, include/, slices/ or syms.txt. Output lands in
  tools/auto_decomp/work/<unit>/ for a human (or a stronger model) to land. A
  matched *function* is not a matched *TU* -- whole-TU-or-nothing means the only
  real gate is `ninja && python progress.py --verify-bin`, which this does not run.
* The sweeper runs BEFORE the model on retries. Most register-allocation misses
  in this project were fixed by mechanical source permutations we already know
  (see HANDOFF.md "Levers"), and enumerating those is far cheaper than a token.
* Escalation is deliberate: cheap/local model first, expensive model only when
  the cheap one plateaus, because plateaus are the signal that a coupled change
  is needed and that is where reasoning actually pays.
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HERE = os.path.dirname(os.path.abspath(__file__))
WORK = os.path.join(HERE, 'work')

MWCC = os.path.join(ROOT, 'compilers', 'Wii', '1.1', 'mwcceppc.exe')
DTK = os.path.join(ROOT, 'bin', 'dtk-windows-x86_64.exe')

CFLAGS = ['-c', '-proc', 'gekko', '-fp', 'hard', '-O4', '-inline', 'noauto',
          '-Cpp_exceptions', 'off', '-enum', 'int', '-RTTI', 'off', '-ipa', 'file',
          '-enc', 'SJIS', '-DREVOLUTION', '-I-']
INCLUDES = ['include', 'include/lib', 'include/lib/MSL', 'include/lib/MSL/internal',
            'include/lib/revolution/BTE/include', 'include/lib/revolution/BTE/stack/include',
            'include/lib/revolution/BTE/stack/btm', 'include/lib/revolution/BTE/bta/include',
            'include/lib/revolution/BTE/bta/sys', 'include/lib/revolution/BTE/gki/common',
            'include/lib/revolution/BTE/gki/platform']

# Compiler-pool symbols get fresh numbers every run, so they cannot be compared
# literally. The original binary names one `@71831_8042B7EC` -- symbol plus dtk's
# address suffix -- where a freshly compiled object has a bare `@21389`. Match the
# combined form first so a reference stays ONE token on both sides.
POOL_SYM = re.compile(r'@\d+(?:_[0-9A-Fa-f]{8})?'
                      r'|lbl_[0-9A-Fa-f]{8}'
                      r'|\.\.\.(?:data|rodata|bss|sdata2?)\.\d+')

# dtk also appends _<ADDR> to ordinary symbols to disambiguate duplicate names.
# Applied AFTER pool numbering, so it cannot eat an `lbl_########`.
ADDR_SUFFIX_INLINE = re.compile(r'_[0-9A-Fa-f]{8}\b')

# ...but `fn_########` / `func_########` are dtk PLACEHOLDER names for functions
# with no symbol, where the address IS the whole name. Stripping the suffix from
# an operand turns every unnamed callee into the bare token `fn`, so
# `bl fn_800A1234` and `bl fn_800CDEF0` compare EQUAL -- a wrong callee passing
# as a match. Fixing norm_name() alone did not cover this: that fixed which
# function gets looked up, this is about what the compared TEXT says.
PLACEHOLDER_CALLEE = re.compile(r'\b(fn|func)_([0-9A-Fa-f]{8})\b')
_KEEP = '\x00'


def canonicalise(lines):
    """Rewrite pool references so the two sides are comparable.

    An earlier version rewrote every pool symbol to one shared marker. That was
    wrong twice over. It reported a spurious difference on every .sdata2
    reference, because the target's two-part name produced two markers against a
    draft's one -- two functions that were in fact byte-exact were reported as
    failing. And it erased *which* literal was referenced, so 0.0f and 8.0f
    compared equal and a wrong constant could pass.

    Numbering each distinct symbol by first appearance, per side, fixes both:
    the naming still cannot leak in, but "the same literal twice" stays
    distinguishable from "two different literals".

    A caveat survives and callers must respect it: this proves the *pattern* of
    references matches, not that the underlying constants are equal. Two
    functions each referencing a single distinct float compare equal whatever
    those floats are. Read the emitted value out of the object and check it
    against the target's pool slot before trusting a match.
    """
    mapping = {}
    out = []
    for line in lines:
        def number(m):
            return mapping.setdefault(m.group(0), 'SYM%d' % len(mapping))
        s = POOL_SYM.sub(number, line)
        # Shield placeholder callee names from the suffix strip below; their
        # address is their identity, not a disambiguator.
        s = PLACEHOLDER_CALLEE.sub(lambda m: m.group(1) + _KEEP + m.group(2), s)
        s = ADDR_SUFFIX_INLINE.sub('', s)
        out.append(s.replace(_KEEP, '_'))
    return out


# ---------------------------------------------------------------- build & diff

def compile_draft(src, obj, extra_inc=()):
    args = [MWCC] + CFLAGS + [src, '-o', obj]
    for inc in list(extra_inc) + INCLUDES:
        args += ['-i', inc.replace('/', os.sep)]
    p = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')


def disasm(obj, out):
    p = subprocess.run([DTK, 'elf', 'disasm', obj, out], cwd=ROOT,
                       capture_output=True, text=True)
    return p.returncode == 0, (p.stdout or '') + (p.stderr or '')


# dtk emits `.fn "name", global` ... `.endfn "name"`, with instruction lines of the
# form `/* ADDR OFFSET  BYTES */\tinsn`. It also appends _<ADDR> to disambiguate
# duplicate names, and emits gap_* pseudo-functions for padding.
FN_START = re.compile(r'^\.fn\s+"?(.+?)"?\s*,\s*\w+\s*$')
FN_END = re.compile(r'^\.endfn\b')
INSN = re.compile(r'^/\*.*?\*/\s*(\S.*)$')
ADDR_SUFFIX = re.compile(r'_[0-9A-Fa-f]{8}$')

# Same line, but keeping the raw instruction word. Needed only for local
# branches -- see LOCAL_BRANCH below.
INSN_WORD = re.compile(r'^/\*\s*\S+\s+\S+\s+'
                       r'((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})'
                       r'\s*\*/\s*(\S.*)$')

# A branch to a local label. The label NAME is useless for comparison -- the
# target names them by absolute address (`.L_8005DB2C`), a fresh object by
# section offset (`.L_00000B38`) -- and canonicalise() reduces both to a bare
# `.L`. That erased control flow entirely: two functions whose branches went to
# DIFFERENT places compared equal, so every loop, conditional and switch in the
# project was being diffed blind. Found by an authoring agent, confirmed by
# direct test.
#
# The fix is to compare the branch's raw instruction word instead. Local
# branches are PC-relative and carry no relocation, so identical code always
# produces an identical word on both sides -- it is an exact check, not a
# heuristic, and it needs no label bookkeeping.
LOCAL_BRANCH = re.compile(r'\.L_[0-9A-Fa-f]{8}\b')


# Placeholder names dtk invents for functions with no symbol: the whole name is
# the prefix plus the address, so stripping the address suffix would leave a
# bare "fn"/"lbl"/"func" that collides with every other unnamed function in the
# file. That is not a missed lookup -- extract() would silently return the FIRST
# unnamed body in the file and diff it against yours, which reads as a real
# result. d_a_en_blockmain.cpp has ten unnamed functions, and every diff against
# one of them was comparing the wrong function.
PLACEHOLDER_FN = re.compile(r'^(?:fn|lbl|func)_[0-9A-Fa-f]{8}$')


def norm_name(n):
    n = n.strip().strip('"')
    # dtk disambiguates duplicate symbol names by appending _<addr>; strip that.
    # But never strip it from a placeholder name, where the address IS the name.
    if PLACEHOLDER_FN.match(n):
        return n
    return ADDR_SUFFIX.sub('', n)


SIZE_HINT = re.compile(r'size:\s*(0x[0-9A-Fa-f]+|\d+)')


def list_functions(path, with_size=False):
    """Function names in target order; optionally with their byte size.

    dtk precedes each .fn with a comment carrying the size, e.g.
        # .text:0x0 | 0x800331E0 | size: 0xC
    """
    out, pending = [], None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            if s.startswith('#'):
                m = SIZE_HINT.search(s)
                if m:
                    pending = int(m.group(1), 16 if m.group(1).startswith('0x') else 10)
                continue
            m = FN_START.match(s)
            if m and not m.group(1).startswith('gap_'):
                out.append((norm_name(m.group(1)), pending or 0) if with_size
                           else norm_name(m.group(1)))
                pending = None
    return out


CLASS_TAG = re.compile(r'__(\d+)([A-Za-z_0-9]+)')


def owning_class(names):
    """The class most of these functions belong to.

    prepare.py collects whole split objects, and those are not TU-aligned, so a
    target usually carries a few functions from a neighbour. Those can never
    match from this draft, and grinding them would burn the whole budget on
    something structurally impossible.
    """
    counts = {}
    for n in names:
        m = CLASS_TAG.search(n)
        if m:
            cls = m.group(2)[:int(m.group(1))]
            counts[cls] = counts.get(cls, 0) + 1
    return max(counts, key=counts.get) if counts else None


def in_scope(name, cls):
    if not cls:
        return True
    m = CLASS_TAG.search(name)
    return cls in name if not m else m.group(2)[:int(m.group(1))] == cls or cls in name


def extract(path, name):
    """Pull one function's instruction lines out of a dtk disassembly.

    Returns the FIRST function whose normalised name matches. If more than one
    function in the file normalises to the same name that is ambiguous, and
    silently taking the first is how a whole batch of diffs once compared the
    wrong body -- so warn loudly rather than pick.
    """
    if not os.path.exists(path):
        return None
    want = norm_name(name)
    # Pre-pass: a name that matches more than one function is ambiguous, and
    # extract() returns at the first match, so the collision is otherwise
    # invisible. Must be counted before extracting, not during.
    hits = 0
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = FN_START.match(line.strip())
            if m and norm_name(m.group(1)) == want:
                hits += 1
    if hits > 1:
        sys.stderr.write(
            'harness: WARNING: %r matches %d functions in %s -- returning the '
            'first. Select by address instead; a silent wrong-function compare '
            'reads exactly like a real result.\n' % (name, hits, path))
    body = None
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            s = line.strip()
            m = FN_START.match(s)
            if m:
                body = [] if norm_name(m.group(1)) == want else None
                continue
            if FN_END.match(s):
                if body is not None:
                    return canonicalise(body) if body else None
                continue
            if body is not None:
                mw = INSN_WORD.match(s)
                if mw and LOCAL_BRANCH.search(mw.group(2)):
                    # Keep the raw word so the branch displacement is compared.
                    word = ''.join(mw.group(1).split())
                    body.append('%s |%s|' % (mw.group(2).strip(), word))
                    continue
                mi = INSN.match(s)
                if mi:
                    body.append(mi.group(1).strip())
    return canonicalise(body) if body else None


def diff_fn(target_txt, draft_txt, name):
    """(matched, human-readable report). Hard-fails if either side is missing."""
    want, got = extract(target_txt, name), extract(draft_txt, name)
    if want is None:
        return False, 'TARGET MISSING: %s not found in %s' % (name, target_txt)
    if got is None:
        return False, 'DRAFT MISSING: %s not emitted' % name
    if want == got:
        msg = 'MATCHING (%d instructions)' % len(want)
        if any('SYM' in line for line in want):
            msg += ('\n  NOTE: this function references pooled literals. Their names cannot be\n'
                    '  compared directly, so the check is on the PATTERN of references, not on\n'
                    '  the values -- a lone 0.0f and a lone 8.0f compare equal here. Read the\n'
                    '  emitted value out of your object and check it against the target slot.')
        return True, msg
    lines = ['size: target %d, draft %d' % (len(want), len(got))]
    for i in range(max(len(want), len(got))):
        a = want[i] if i < len(want) else '<none>'
        b = got[i] if i < len(got) else '<none>'
        if a != b:
            lines.append('  %3d | want: %-44s got: %s' % (i, a, b))
        if len(lines) > 40:
            lines.append('  ... truncated')
            break
    return False, '\n'.join(lines)


# ---------------------------------------------------------------- deterministic sweep

def sweep_variants(src_text, fn_name):
    """Mechanical permutations of the levers HANDOFF.md documents.

    Cheap to try, no tokens, and they fixed the majority of this project's
    register-allocation misses. Returns (label, source) pairs.
    """
    out = []
    # eager sum -> compound assignment (a reassociation barrier; closed d_a_en_door)
    for m in re.finditer(r'(\w[\w:\s\*&]*?)\b(\w+)\s*=\s*([^;]+?)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*;',
                         src_text):
        decl, var, base, k = m.groups()
        out.append(('compound:%s' % var,
                    src_text[:m.start()] + '%s%s = %s;\n    %s += %s;' % (decl, var, base, var, k) +
                    src_text[m.end():]))
    # widen/narrow an intermediate: 16-bit stops MWCC reassociating
    # NOTE: no f32<->double here on purpose. That changes the arithmetic, not
    # just the allocation, so it is not a lever -- it is a different program
    # that happens to compile. Every other pair preserves the computed value.
    for a, b in (('int ', 's16 '), ('s16 ', 'int '), ('u32 ', 'int '), ('int ', 'u32 '),
                 ('u16 ', 's16 '), ('s16 ', 'u16 ')):
        if a in src_text:
            out.append(('retype:%s->%s' % (a.strip(), b.strip()), src_text.replace(a, b, 1)))
    # float division vs multiply (e.g. / 2.0f -> * 0.5f)
    for a, b in (('/ 2.0f', '* 0.5f'), ('* 0.5f', '/ 2.0f'), ('/ 2.0', '* 0.5'), ('* 0.5', '/ 2.0')):
        if a in src_text:
            out.append(('fmul-div:%s->%s' % (a, b), src_text.replace(a, b, 1)))
    # helper call -> direct field stores (inlined parameter binding moves FPRs)
    m = re.search(r'(\w+)\.set\(([^,]+),\s*([^)]+)\);', src_text)
    if m:
        obj, x, y = m.groups()
        out.append(('fieldstores:%s' % obj,
                    src_text[:m.start()] + '%s.x = %s;\n    %s.y = %s;' % (obj, x, obj, y) +
                    src_text[m.end():]))
    return out


# ---------------------------------------------------------------- model client

def model_keys(cfg):
    """Every model defined in the config, in file order.

    Any dict-valued key that does not start with '_' is a model, so adding one
    to config.json is all that is needed -- nothing here is hardcoded.
    """
    return [k for k, v in cfg.items() if not k.startswith('_') and isinstance(v, dict)]


def ladder_of(cfg):
    """Escalation order: cheapest first, last resort last.

    Defaults to the order the models appear in config.json, so you control the
    ladder by ordering the file. Override with an explicit "_ladder" list if you
    want a different order from the definition order.
    """
    keys = model_keys(cfg)
    declared = cfg.get('_ladder')
    if declared:
        unknown = [k for k in declared if k not in keys]
        if unknown:
            sys.exit('_ladder names models that are not in the config: %s' % ', '.join(unknown))
        return list(declared)
    return keys


def ask(cfg, messages):
    """OpenAI-compatible chat completion. Works for llama.cpp/LM Studio/Ollama
    and for Gemini via its OpenAI-compatible endpoint."""
    body = json.dumps({'model': cfg['model'], 'messages': messages,
                       'temperature': cfg.get('temperature', 0.3),
                       'max_tokens': cfg.get('max_tokens', 4096)}).encode()
    req = urllib.request.Request(
        cfg['base_url'].rstrip('/') + '/chat/completions', data=body,
        headers={'Content-Type': 'application/json',
                 'Authorization': 'Bearer ' + cfg.get('api_key', 'none')})
    with urllib.request.urlopen(req, timeout=cfg.get('timeout', 180)) as r:
        return json.loads(r.read())['choices'][0]['message']['content']


def code_of(reply):
    """Extract a COMPLETE fenced C++ block, or nothing.

    Local models often wrap reasoning in <thought> tags or run out of tokens
    mid-block. Returning a truncated fragment is worse than returning nothing:
    it compiles to garbage, or worse, compiles to something plausible. Every
    fallback here must yield a whole block or give up.
    """
    if not reply:
        return ''
    clean = re.sub(r'<thought>.*?</thought>', '', reply, flags=re.S)
    clean = re.sub(r'<think>.*?</think>', '', clean, flags=re.S).strip()

    # A closed fence is the only trustworthy form. Prefer the reasoning-stripped
    # text, then the raw reply in case the tags were themselves unbalanced.
    for text in (clean, reply):
        blocks = re.findall(r'```(?:cpp|c\+\+)?\s*\n(.*?)```', text, re.S)
        if blocks:
            return blocks[-1].strip()

    # No fence at all: accept only if it looks like a complete, balanced unit.
    if clean and clean.count('{') and clean.count('{') == clean.count('}'):
        return clean

    # Deliberately no "grab everything after an unclosed fence" fallback: that
    # is how a token-limited reply becomes a silently truncated function.
    return ''


def merge_code(old_source, new_reply, fn_hint=None):
    """Fold the model's code into the draft. Returns (source, what_happened).

    Three cases, in order: a whole file replaces the draft; a definition that
    already exists is replaced in place; anything else is appended once.

    Two bugs worth not reintroducing. Using re.sub to splice the reply in makes
    the replacement an escape-interpreted template, so any backslash in the C++
    (a '\\n' in a string literal, say) is silently mangled -- splice literally
    instead. And appending unconditionally means a model that keeps failing
    grows the draft without bound until nothing compiles; append only when the
    definition is genuinely absent.
    """
    code = code_of(new_reply)
    if not code:
        return old_source, 'no usable code block -- draft left unchanged'

    if '#include' in code and ('class ' in code or 'struct ' in code):
        return code, 'replaced whole file'

    m = re.search(r'\b([\w~]+(?:::[\w~]+)?)\s*\([^;{]*\)\s*(?:const\s*)?\{', code)
    if m:
        sig = m.group(1)
        existing = re.compile(
            r'^[\w:\*&<>,\s]*?\b' + re.escape(sig) + r'\s*\([^;{]*\)\s*(?:const\s*)?\{.*?^\}',
            re.M | re.S)
        found = existing.search(old_source)
        if found:
            # Literal splice: never let re treat the C++ as a replacement template.
            return (old_source[:found.start()] + code + old_source[found.end():],
                    'replaced %s in place' % sig)

    if code.strip() and code.strip() in old_source:
        return old_source, 'model returned code already present -- draft left unchanged'

    return old_source.rstrip() + '\n\n' + code + '\n', 'appended new definition'


SYSTEM = """You reconstruct C++ that CodeWarrior 1.1 compiles to byte-identical PowerPC.

Non-negotiable rules for this codebase:
- Same instructions, same order, same REGISTERS. Registers differing is a failure.
- Never invent a member offset. If unsure, say so instead of guessing.
- USE NAMED MEMBERS. `mParentID = parent->mUniqueID;` -- never
  `*(u32*)((u8*)this + 0x3d0) = *(u32*)parent;`. A raw offset cast will always
  reproduce the bytes, which is exactly why it is not an answer: it matches
  without explaining anything, and it is unreadable to the next person. Reach
  for a cast ONLY when the field belongs to a class that has genuinely not been
  decompiled, and then say so in a one-line comment.
- Ship no reasoning in the source. No "likely", "seems odd", "let's assume". If
  you are unsure, put it in prose OUTSIDE the code block. Comments in the code
  are for load-bearing facts only.
- Empty virtuals must be defined out of line, never in the class body.
- A function-scope `static const int` allocates storage; use an enum instead.

Levers that repeatedly fix "instructions right, registers wrong":
- Add one named local holding a value already computed. Costs no instructions.
- Compound form `x = a; x += K;` instead of `x = a + K;`.
- Narrow an intermediate to s16/mAng: MWCC will not reassociate across it.
- Direct field stores `v.x = a; v.y = b;` instead of `v.set(a, b)`.
- Declaration order sets GPR allocation; FPR direction is NOT fixed, sweep it.

If a single change plateaus, the answer is almost always TWO changes applied
together. Four functions in this project needed a coupled pair after sweeps of
25, 115, 120 and 300 single-variable attempts had each stalled.

Keep any reasoning short. Reply with ONE complete fenced C++ block and nothing
else -- a truncated block is worse than no answer, because it may still
compile."""


def format_report_summary(report, max_lines=6):
    """Format instruction diff or compiler error for clean terminal display."""
    if not report:
        return '    (no report)'
    lines = report.splitlines()
    out = []
    for l in lines[:max_lines]:
        out.append('    ' + l)
    if len(lines) > max_lines:
        out.append('    ... (+%d more lines)' % (len(lines) - max_lines))
    return '\n'.join(out)


def preview_code(code_str, max_lines=8):
    """Preview snippet of code generated by LLM."""
    if not code_str:
        return '    (empty)'
    lines = [l for l in code_str.splitlines() if l.strip()]
    out = []
    for l in lines[:max_lines]:
        out.append('    ' + l)
    if len(lines) > max_lines:
        out.append('    ... (+%d lines)' % (len(lines) - max_lines))
    return '\n'.join(out)


# ---------------------------------------------------------------- driver

def run(cfg, unit, fn, target_txt, workdir, budget, model_mode='auto'):
    src_path = os.path.join(workdir, 'draft.cpp')
    if not os.path.exists(src_path):
        sys.exit('no draft at %s -- seed it with the class scaffold first' % src_path)
    obj = os.path.join(workdir, 'draft.o')
    txt = os.path.join(workdir, 'draft.txt')
    history = []
    ladder = ladder_of(cfg)
    rungs_climbed = 0

    print('\n' + '=' * 78)
    print(' TARGET: %s' % fn)
    print(' UNIT  : %s' % unit)
    print('=' * 78)

    for attempt in range(1, budget + 1):
        src = open(src_path, encoding='utf-8').read()
        ok, log = compile_draft(src_path, obj)
        report = None
        matched = False

        if not ok:
            err_line = next((l.strip() for l in log.splitlines() if 'Error:' in l or '(10' in l), log.splitlines()[-1] if log else 'Compile failed')
            report = 'Compile Error: %s\n\nFull log:\n%s' % (err_line, log.strip()[:1000])
        else:
            dok, dlog = disasm(obj, txt)
            if not dok:
                report = 'disasm failed: %s' % dlog[:400]
            else:
                matched, report = diff_fn(target_txt, txt, fn)
                if matched:
                    print('\n' + '*' * 78)
                    print(' [Attempt %d/%d] MATCH! %s' % (attempt, budget, fn))
                    print('*' * 78)
                    open(os.path.join(workdir, 'MATCHED.cpp'), 'w',
                         encoding='utf-8').write(src)
                    return True

        print('\n[Attempt %d/%d]' % (attempt, budget))
        print(' - Status:\n%s' % format_report_summary(report, max_lines=8))

        # cheap mechanical sweep before spending a token
        for label, variant in sweep_variants(src, fn):
            open(src_path, 'w', encoding='utf-8').write(variant)
            vok, _ = compile_draft(src_path, obj)
            if vok and disasm(obj, txt)[0] and diff_fn(target_txt, txt, fn)[0]:
                print(' - Mechanical sweep matched via: %s' % label)
                print('\n' + '*' * 78)
                print(' [Attempt %d/%d] MATCH! %s' % (attempt, budget, fn))
                print('*' * 78)
                open(os.path.join(workdir, 'MATCHED.cpp'), 'w',
                     encoding='utf-8').write(variant)
                return True
        open(src_path, 'w', encoding='utf-8').write(src)  # restore

        want = extract(target_txt, fn) or []
        history.append(report)
        stuck = len(history) >= 3 and len(set(history[-3:])) == 1

        if model_mode != 'auto':
            which_key = model_mode
        else:
            # Walk the ladder: start at the bottom, climb one rung per plateau,
            # and climb once more past the halfway point of the budget so a
            # slow-but-not-identical failure still reaches a better model.
            rung = rungs_climbed + (1 if attempt > budget // 2 else 0)
            which_key = ladder[min(rung, len(ladder) - 1)]
        which = cfg[which_key]

        if stuck and model_mode == 'auto':
            rungs_climbed = min(rungs_climbed + 1, len(ladder) - 1)
            print(' - Plateau -> escalating to [%s: %s] with a COUPLED prompt...'
                  % (which_key, which['model']))
        else:
            print(' - Querying [%s: %s]...' % (which_key, which['model']))

        msgs = [{'role': 'system', 'content': SYSTEM},
                {'role': 'user', 'content':
                 'Target disassembly of %s:\n%s\n\nCurrent source:\n```cpp\n%s\n```\n\n'
                 'Result:\n%s\n\n%s' % (
                     fn, '\n'.join(want[:120]), src, report,
                     'You have plateaued: the same difference three times. Apply TWO '
                     'changes together, not one.' if stuck else 'Fix it.')}]
        try:
            reply = ask(which, msgs)
            merged, how = merge_code(src, reply, fn)
            print(' - LLM output (%d lines) -> %s' % (
                len(code_of(reply).splitlines()), how))
            print(preview_code(code_of(reply), max_lines=6))
            if merged == src:
                # Nothing usable came back. Retrying the same prompt against the
                # same model will just burn the budget, so escalate immediately.
                print(' - No change to the draft; forcing escalation next attempt.')
                history.append('UNUSABLE-REPLY-%d' % attempt)
            open(src_path, 'w', encoding='utf-8').write(merged)
        except Exception as exc:
            print(' - Model Error [%s]: %s' % (which['model'], exc))
            return False
    print('\n[Finished] Reached budget limit (%d attempts) without closing %s.\n' % (budget, fn))
    return False


def log_event(workdir, **fields):
    """Append one line to the audit trail. Every function the harness closes,
    every one it gives up on, and every landing attempt is recorded here."""
    path = os.path.join(workdir, 'decomp_log.jsonl')
    fields.setdefault('at', __import__('datetime').datetime.now().isoformat(timespec='seconds'))
    with open(path, 'a', encoding='utf-8') as fh:
        fh.write(json.dumps(fields) + '\n')
    return path


def auto_mode(cfg, unit, target_txt, workdir, budget, model_mode, land_with=None):
    """Grind a whole TU smallest-function-first, then try to land it.

    Smallest first is deliberate: the little ones are boilerplate and accessors,
    they close fast, and each one that lands makes the file more correct for the
    harder ones that follow. It also front-loads the evidence about whether this
    model can do the job at all, before spending a long budget finding out.
    """
    src_path = os.path.join(workdir, 'draft.cpp')
    if not os.path.exists(src_path):
        sys.exit('no draft at %s -- seed it first' % src_path)
    obj, txt = os.path.join(workdir, 'draft.o'), os.path.join(workdir, 'draft.txt')

    sized = list_functions(target_txt, with_size=True)
    cls = owning_class([n for n, _ in sized])
    scoped = [(n, s) for n, s in sized if in_scope(n, cls)]
    skipped = [n for n, _ in sized if not in_scope(n, cls)]
    if skipped:
        print('Ignoring %d function(s) from other TUs in this split object:' % len(skipped))
        for n in skipped[:6]:
            print('   - %s' % n)
        log_event(workdir, event='out_of_scope', unit=unit, functions=skipped)

    print('\nTarget class: %s   %d functions in scope' % (cls, len(scoped)))
    log_event(workdir, event='auto_start', unit=unit, owning_class=cls,
              in_scope=len(scoped), model=model_mode, budget=budget)

    closed, failed = [], []
    order = sorted(scoped, key=lambda p: (p[1], p[0]))  # smallest first
    for idx, (fn, size) in enumerate(order, 1):
        already = False
        if compile_draft(src_path, obj)[0] and disasm(obj, txt)[0]:
            already, _ = diff_fn(target_txt, txt, fn)
        if already:
            print('[%d/%d] already matching (%d B) %s' % (idx, len(order), size, fn))
            closed.append(fn)
            continue
        print('\n[%d/%d] %s  (%d bytes)' % (idx, len(order), fn, size))
        before = open(src_path, encoding='utf-8').read()
        ok = run(cfg, unit, fn, target_txt, workdir, budget, model_mode=model_mode)
        if ok:
            closed.append(fn)
            # run() writes MATCHED.cpp; promote it so the next function builds on it
            matched = os.path.join(workdir, 'MATCHED.cpp')
            if os.path.exists(matched):
                shutil.copyfile(matched, src_path)
            log_event(workdir, event='closed', unit=unit, function=fn, size=size)
        else:
            failed.append(fn)
            open(src_path, 'w', encoding='utf-8').write(before)  # never keep a failed draft
            log_event(workdir, event='gave_up', unit=unit, function=fn, size=size)

    print('\n%s\n %d closed, %d not closed, of %d in scope' %
          ('=' * 78, len(closed), len(failed), len(scoped)))
    log_event(workdir, event='auto_done', unit=unit,
              closed=len(closed), failed=len(failed), total=len(scoped))

    if failed:
        print('\nNot landing: a TU only counts when EVERY function matches.')
        print('Unclosed: %s' % ', '.join(failed[:8]))
        return False

    print('\nAll functions match. Attempting to land behind the build gate...')
    return attempt_land(unit, workdir, land_with)


def attempt_land(unit, workdir, land_with):
    """Hand the finished draft to land.py, which is the only thing that writes
    to the project -- and only if the full build plus a five-binary hash check
    passes. Anything less and it restores every file it touched."""
    if not land_with:
        print('No --land-with slice ranges given, so stopping here.')
        print('Draft is at %s/draft.cpp' % workdir)
        log_event(workdir, event='land_skipped', unit=unit, reason='no slice ranges')
        return False
    cmd = [sys.executable, os.path.join(HERE, 'land.py'), '--unit', unit,
           '--cpp', os.path.join(workdir, 'draft.cpp'), '--slice', land_with]
    print('  $ ' + ' '.join(cmd[1:]))
    p = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    out = (p.stdout or '') + (p.stderr or '')
    print(out.strip()[-1200:])
    log_event(workdir, event='land_result', unit=unit, accepted=(p.returncode == 0),
              output=out.strip()[-600:])
    return p.returncode == 0


def test_endpoint(cfg, model_key):
    """Test connectivity to a specific model endpoint in config.json."""
    if model_key not in cfg:
        sys.exit('Model key "%s" not found in config.json (available: %s)' % (
            model_key, ', '.join(k for k in cfg if not k.startswith('_'))))
    target_cfg = cfg[model_key]
    print('Testing connection to [%s]...' % model_key)
    print('  Base URL : %s' % target_cfg.get('base_url'))
    print('  Model    : %s' % target_cfg.get('model'))
    try:
        reply = ask(target_cfg, [{'role': 'user', 'content': 'Respond with only the word: ONLINE'}])
        print('  Status   : SUCCESS!')
        print('  Response : %s\n' % reply.strip())
        return True
    except Exception as exc:
        print('  Status   : FAILED (%s)\n' % exc)
        return False


def check_status(target_txt, workdir):
    """Check how many functions in target currently match in draft.cpp."""
    src_path = os.path.join(workdir, 'draft.cpp')
    if not os.path.exists(src_path):
        sys.exit('no draft at %s' % src_path)
    obj = os.path.join(workdir, 'draft.o')
    txt = os.path.join(workdir, 'draft.txt')

    ok, log = compile_draft(src_path, obj)
    if not ok:
        print('draft.cpp failed to compile:\n%s' % log[:600])
        return

    dok, dlog = disasm(obj, txt)
    if not dok:
        print('disasm failed:\n%s' % dlog[:600])
        return

    fns = list_functions(target_txt)
    matched_count = 0
    print('Checking %d functions against %s:' % (len(fns), target_txt))
    for i, fn in enumerate(fns):
        matched, rep = diff_fn(target_txt, txt, fn)
        tag = 'OK ' if matched else 'NO '
        if matched:
            matched_count += 1
        print('  [%2d/%2d] %s %s' % (i + 1, len(fns), tag, fn))
    print('\nStatus: %d / %d functions matching (%.1f%%)' % (
        matched_count, len(fns), (matched_count / len(fns) * 100.0) if fns else 0.0))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--unit', help='e.g. dol/bases/d_a_foo.cpp')
    ap.add_argument('--fn', help='mangled function name to close')
    ap.add_argument('--model', default='auto',
                    help='pin to one model named in config.json, or "auto" (default) '
                         'to start at the bottom of the ladder and escalate on a plateau')
    ap.add_argument('--test-model',
                    help='test endpoint connectivity for one model, or "all"')
    ap.add_argument('--list-models', action='store_true',
                    help='show the models in config.json and the escalation order')
    ap.add_argument('--all', action='store_true', help='run all unfinished functions in target')
    ap.add_argument('--auto', action='store_true',
                    help='grind the whole TU smallest-function-first, log every '
                         'result, and try to land it once every function matches')
    ap.add_argument('--land-with', metavar='JSON',
                    help='slice memoryRanges, e.g. \'{".text": "0x1000-0x2000"}\'. '
                         'Required for --auto to land; without it, it stops at the draft.')
    ap.add_argument('--status', action='store_true', help='check match status of all functions in draft')
    ap.add_argument('--target', help='target disassembly txt (default: work/<unit>/target.txt)')
    ap.add_argument('--list', action='store_true', help='list functions in the target and exit')
    ap.add_argument('--budget', type=int, default=12, help='model attempts (default 12)')
    ap.add_argument('--config', default=os.path.join(HERE, 'config.json'))
    args = ap.parse_args()

    if not os.path.exists(args.config):
        sys.exit('no config at %s -- copy config.example.json and fill it in' % args.config)
    cfg = json.load(open(args.config, encoding='utf-8'))

    known = model_keys(cfg)
    if not known:
        sys.exit('no models in %s -- a model is any dict-valued key not starting with "_"'
                 % args.config)

    if args.list_models:
        ladder = ladder_of(cfg)
        print('Models in %s:' % os.path.basename(args.config))
        for k in known:
            v = cfg[k]
            print('  %-10s %-18s %s' % (k, v.get('model', '?'), v.get('base_url', '?')))
        print('\nEscalation order (--model auto): %s' % ' -> '.join(ladder))
        print('Set "_ladder": [...] in the config to change it; otherwise it is file order.')
        return

    if args.test_model:
        targets = known if args.test_model == 'all' else [args.test_model]
        if args.test_model != 'all' and args.test_model not in known:
            sys.exit('unknown model "%s" -- config has: %s' % (args.test_model, ', '.join(known)))
        ok = all([test_endpoint(cfg, k) for k in targets])
        sys.exit(0 if ok else 1)

    if args.model != 'auto' and args.model not in known:
        sys.exit('unknown model "%s" -- config has: %s (or "auto")'
                 % (args.model, ', '.join(known)))

    if not args.unit:
        sys.exit('--unit is required (e.g. --unit dol/bases/d_a_en_lkuribo_base.cpp)')

    slug = args.unit.replace('/', '_').replace('.cpp', '')
    workdir = os.path.join(WORK, slug)
    os.makedirs(workdir, exist_ok=True)
    target = args.target or os.path.join(workdir, 'target.txt')

    if args.list:
        if not os.path.exists(target):
            sys.exit('no target disassembly at %s' % target)
        fns = list_functions(target)
        print('Functions in target (%d total):' % len(fns))
        for i, fn in enumerate(fns):
            print('  [%2d] %s' % (i + 1, fn))
        return

    if args.status:
        if not os.path.exists(target):
            sys.exit('no target disassembly at %s' % target)
        check_status(target, workdir)
        return

    if args.auto:
        if not os.path.exists(target):
            sys.exit('no target disassembly at %s' % target)
        ok = auto_mode(cfg, args.unit, target, workdir, args.budget,
                       args.model, args.land_with)
        print('\nAudit trail: %s' % os.path.join(workdir, 'decomp_log.jsonl'))
        sys.exit(0 if ok else 1)

    if args.all:
        if not os.path.exists(target):
            sys.exit('no target disassembly at %s' % target)
        fns = list_functions(target)
        txt = os.path.join(workdir, 'draft.txt')
        # Compile once up front so "already matching" is judged against the
        # CURRENT draft. Reusing a stale draft.txt from an earlier run can skip
        # a function that no longer matches.
        obj = os.path.join(workdir, 'draft.o')
        src_path = os.path.join(workdir, 'draft.cpp')
        fresh = False
        if os.path.exists(src_path) and compile_draft(src_path, obj)[0]:
            fresh = disasm(obj, txt)[0]
        if not fresh and os.path.exists(txt):
            os.remove(txt)  # refuse to trust a stale disassembly
        for fn in fns:
            matched = False
            if os.path.exists(txt):
                matched, _ = diff_fn(target, txt, fn)
            if matched:
                print('SKIP (already matching): %s' % fn)
                continue
            print('\n=== Running function: %s [Model: %s] ===' % (fn, args.model))
            run(cfg, args.unit, fn, target, workdir, args.budget, model_mode=args.model)
        return

    if not args.fn:
        sys.exit('--fn is required (or use --list, --status, or --all)')

    ok = run(cfg, args.unit, args.fn, target, workdir, args.budget, model_mode=args.model)
    print('\n%s' % ('MATCHED -- review %s/MATCHED.cpp, then land it by hand'
                    % workdir if ok else 'not closed; best diff above'))
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()

