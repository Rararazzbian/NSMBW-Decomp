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

# Compiler-pool symbols get fresh numbers every run; they are not real differences.
POOL = re.compile(r'@\d+|\.\.\.(?:data|rodata|bss|sdata2?)\.\d+|lbl_[0-9A-Fa-f]{8}|_[0-9A-Fa-f]{8}\b')


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


def norm_name(n):
    return ADDR_SUFFIX.sub('', n.strip().strip('"'))


def list_functions(path):
    out = []
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = FN_START.match(line.strip())
            if m and not m.group(1).startswith('gap_'):
                out.append(norm_name(m.group(1)))
    return out


def extract(path, name):
    """Pull one function's instruction lines out of a dtk disassembly."""
    if not os.path.exists(path):
        return None
    want = norm_name(name)
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
                    return body or None
                continue
            if body is not None:
                mi = INSN.match(s)
                if mi:
                    body.append(POOL.sub('@P', mi.group(1).strip()))
    return body or None


def diff_fn(target_txt, draft_txt, name):
    """(matched, human-readable report). Hard-fails if either side is missing."""
    want, got = extract(target_txt, name), extract(draft_txt, name)
    if want is None:
        return False, 'TARGET MISSING: %s not found in %s' % (name, target_txt)
    if got is None:
        return False, 'DRAFT MISSING: %s not emitted' % name
    if want == got:
        return True, 'MATCHING (%d instructions)' % len(want)
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

        if model_mode in ('cheap', 'strong'):
            which_key = model_mode
            which = cfg[which_key]
        else:
            which_key = 'strong' if (stuck or attempt > budget // 2) else 'cheap'
            which = cfg[which_key]

        if stuck and model_mode == 'auto':
            print(' - Escalation: Plateau detected -> querying [%s: %s] with COUPLED prompt...' % (which_key, which['model']))
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
    ap.add_argument('--model', choices=['auto', 'cheap', 'strong'], default='auto',
                    help='isolate to a single model: "cheap" (local), "strong" (gemini), or "auto" (default: cheap with auto-escalation)')
    ap.add_argument('--test-model', choices=['cheap', 'strong', 'all'],
                    help='test endpoint connectivity for a model without running a decomp')
    ap.add_argument('--all', action='store_true', help='run all unfinished functions in target')
    ap.add_argument('--status', action='store_true', help='check match status of all functions in draft')
    ap.add_argument('--target', help='target disassembly txt (default: work/<unit>/target.txt)')
    ap.add_argument('--list', action='store_true', help='list functions in the target and exit')
    ap.add_argument('--budget', type=int, default=12, help='model attempts (default 12)')
    ap.add_argument('--config', default=os.path.join(HERE, 'config.json'))
    args = ap.parse_args()

    if not os.path.exists(args.config):
        sys.exit('no config at %s -- copy config.example.json and fill it in' % args.config)
    cfg = json.load(open(args.config, encoding='utf-8'))

    if args.test_model:
        if args.test_model == 'all':
            test_endpoint(cfg, 'cheap')
            test_endpoint(cfg, 'strong')
        else:
            test_endpoint(cfg, args.test_model)
        return

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

