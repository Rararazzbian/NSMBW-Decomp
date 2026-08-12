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


def extract(path, name):
    """Pull one function's instruction lines out of a dtk disassembly."""
    if not os.path.exists(path):
        return None
    body, grabbing = [], False
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            stripped = line.strip().rstrip(':').strip('"')
            if not grabbing:
                if stripped == name or stripped.endswith(' ' + name):
                    grabbing = True
                continue
            if re.match(r'^[A-Za-z_@.$"].*:\s*$', line) and name not in line:
                break
            if '/*' in line or line.strip().startswith('.'):
                text = line.split('*/')[-1].strip()
                if text:
                    body.append(POOL.sub('@P', text))
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
    for a, b in (('int ', 's16 '), ('s16 ', 'int '), ('u32 ', 'int '), ('int ', 'u32 ')):
        if a in src_text:
            out.append(('retype:%s->%s' % (a.strip(), b.strip()), src_text.replace(a, b, 1)))
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
    m = re.search(r'```(?:cpp|c\+\+)?\s*\n(.*?)```', reply, re.S)
    return (m.group(1) if m else reply).strip()


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

If a single change plateaus, the answer is TWO changes applied together.
Reply with one fenced C++ block and nothing else."""


# ---------------------------------------------------------------- driver

def run(cfg, unit, fn, target_txt, workdir, budget):
    src_path = os.path.join(workdir, 'draft.cpp')
    if not os.path.exists(src_path):
        sys.exit('no draft at %s -- seed it with the class scaffold first' % src_path)
    obj = os.path.join(workdir, 'draft.o')
    txt = os.path.join(workdir, 'draft.txt')
    history, best = [], None

    for attempt in range(1, budget + 1):
        src = open(src_path, encoding='utf-8').read()
        ok, log = compile_draft(src_path, obj)
        report = log.strip()[:1500] if not ok else None
        if ok:
            dok, dlog = disasm(obj, txt)
            if not dok:
                report = 'disasm failed: %s' % dlog[:400]
            else:
                matched, report = diff_fn(target_txt, txt, fn)
                if matched:
                    print('[%d] MATCH %s' % (attempt, fn))
                    open(os.path.join(workdir, 'MATCHED.cpp'), 'w',
                         encoding='utf-8').write(src)
                    return True
        print('[%d] %s' % (attempt, report.splitlines()[0] if report else '?'))

        # cheap mechanical sweep before spending a token
        for label, variant in sweep_variants(src, fn):
            open(src_path, 'w', encoding='utf-8').write(variant)
            vok, _ = compile_draft(src_path, obj)
            if vok and disasm(obj, txt)[0] and diff_fn(target_txt, txt, fn)[0]:
                print('    swept -> MATCH via %s' % label)
                open(os.path.join(workdir, 'MATCHED.cpp'), 'w',
                     encoding='utf-8').write(variant)
                return True
        open(src_path, 'w', encoding='utf-8').write(src)  # restore

        want = extract(target_txt, fn) or []
        history.append(report)
        stuck = len(history) >= 3 and len(set(history[-3:])) == 1
        which = cfg['strong'] if (stuck or attempt > budget // 2) else cfg['cheap']
        if stuck:
            print('    plateau -> escalating, asking for a COUPLED change')
        msgs = [{'role': 'system', 'content': SYSTEM},
                {'role': 'user', 'content':
                 'Target disassembly of %s:\n%s\n\nCurrent source:\n```cpp\n%s\n```\n\n'
                 'Result:\n%s\n\n%s' % (
                     fn, '\n'.join(want[:120]), src, report,
                     'You have plateaued: the same difference three times. Apply TWO '
                     'changes together, not one.' if stuck else 'Fix it.')}]
        try:
            open(src_path, 'w', encoding='utf-8').write(code_of(ask(which, msgs)))
        except Exception as exc:
            print('    model error: %s' % exc)
            return False
    return False


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--unit', required=True, help='e.g. dol/bases/d_a_foo.cpp')
    ap.add_argument('--fn', help='mangled function name to close')
    ap.add_argument('--target', help='target disassembly txt (default: work/<unit>/target.txt)')
    ap.add_argument('--list', action='store_true', help='list functions in the target and exit')
    ap.add_argument('--budget', type=int, default=12, help='model attempts (default 12)')
    ap.add_argument('--config', default=os.path.join(HERE, 'config.json'))
    args = ap.parse_args()

    slug = args.unit.replace('/', '_').replace('.cpp', '')
    workdir = os.path.join(WORK, slug)
    os.makedirs(workdir, exist_ok=True)
    target = args.target or os.path.join(workdir, 'target.txt')

    if args.list:
        if not os.path.exists(target):
            sys.exit('no target disassembly at %s' % target)
        with open(target, encoding='utf-8', errors='replace') as fh:
            for line in fh:
                m = re.match(r'^([A-Za-z_][\w@$<>,:]*)\s*:\s*$', line)
                if m:
                    print(m.group(1))
        return

    if not args.fn:
        sys.exit('--fn is required (or use --list)')
    if not os.path.exists(args.config):
        sys.exit('no config at %s -- copy config.example.json and fill it in' % args.config)
    cfg = json.load(open(args.config, encoding='utf-8'))
    ok = run(cfg, args.unit, args.fn, target, workdir, args.budget)
    print('\n%s' % ('MATCHED -- review %s/MATCHED.cpp, then land it by hand'
                    % workdir if ok else 'not closed; best diff above'))
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
