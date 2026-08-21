"""Merge disjoint per-function edits from several parallel agent workspaces.

The parallel-agent pattern used on `d_line_mng.cpp` gives every agent its own
copy of the same draft and assigns it a disjoint set of functions. Merging the
results by hand is where the mistakes happen: `patch` refuses on CRLF-vs-LF
mismatches, and a whole-file diff between two copies is unreadable once two
agents have both reformatted whitespace.

So merge at FUNCTION granularity instead. For each agent copy, work out which
top-level function bodies differ from the common baseline, and splice exactly
those into the live draft. Two agents touching the same function is then a
detectable CONFLICT rather than a silent last-writer-wins.

    python merge_agents.py <live.cpp> <baseline.cpp> <agent1.cpp> [agent2.cpp ...]
    python merge_agents.py --dry-run ...     report what would change, write nothing

Normalises line endings on read, and writes back with the live file's own
convention, because MWCC does not care but `patch` and `git` both do.
"""
import os
import re
import sys

# A top-level definition: starts at column 0, is not a preprocessor line, a
# label, or a closing brace. We do not try to parse C++ -- we only need to find
# the boundaries of things that look like `Type Class::name(args) {`.
DEF_START = re.compile(r'^[A-Za-z_~][\w:<>,*&\s]*\**\s*[\w:~]+\s*\([^;]*$|^[A-Za-z_][\w\s:*&<>,]*::[~\w]+\s*\(')


def strip_noncode(line):
    """Blank out `//` comments and string/char literals before counting braces.

    A brace inside a comment -- and this file's comments quote source code --
    would otherwise unbalance the depth count and run a function's extent off
    the end of the file.
    """
    line = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', '""', line)
    return line.split('//', 1)[0]


def read(path):
    with open(path, 'r', encoding='utf-8', errors='replace', newline='') as f:
        raw = f.read()
    crlf = raw.count('\r\n') > raw.count('\n') // 2
    return raw.replace('\r\n', '\n').split('\n'), crlf


def split_functions(lines):
    """Return {name: (start, end)} for top-level function definitions.

    `end` is exclusive and points past the closing `}` at column 0. Anything not
    inside a function -- includes, file-scope declarations, comments between
    functions -- is deliberately left out; the caller keeps the baseline's copy
    of those, so a stray reformat in one agent copy cannot leak into the merge.
    """
    out = {}
    i, n = 0, len(lines)
    while i < n:
        line = lines[i]
        if not line or line[0] in ' \t#}/' or not DEF_START.match(line):
            i += 1
            continue
        # Walk forward to the `{` that opens the body, bailing if we hit a `;`
        # first (that was a declaration, not a definition).
        j, opened = i, False
        while j < n and j < i + 12:
            if ';' in strip_noncode(lines[j]) and '{' not in strip_noncode(lines[j]):
                break
            if '{' in strip_noncode(lines[j]):
                opened = True
                break
            j += 1
        if not opened:
            i += 1
            continue
        # Find the body's end by BRACE DEPTH, not by looking for a `}` in column
        # 0. A one-line definition -- `void C::finalizeState_X() {}`, of which
        # this file has dozens -- opens and closes on the same line, and a
        # column-0 scan silently swallows every function after it.
        depth, k = 0, j
        while k < n:
            code = strip_noncode(lines[k])
            depth += code.count('{') - code.count('}')
            if depth <= 0:
                break
            k += 1
        if k >= n:
            i += 1
            continue
        name = extract_name(lines[i:j + 1])
        if name:
            out.setdefault(name, (i, k + 1))
        i = k + 1
    return out


def extract_name(head):
    """Pull `Class::method` or a bare function name out of a definition header."""
    text = ' '.join(head)
    text = text[:text.index('(')] if '(' in text else text
    tokens = re.findall(r'[~\w]+(?:::[~\w]+)?', text)
    for tok in reversed(tokens):
        if '::' in tok or tok not in ('const', 'static', 'inline', 'void', 'extern'):
            return tok
    return None


def main():
    argv = [a for a in sys.argv[1:] if a != '--dry-run']
    dry = len(argv) != len(sys.argv[1:])
    if len(argv) < 3:
        print(__doc__)
        return 2
    live_path, base_path, agent_paths = argv[0], argv[1], argv[2:]

    live, live_crlf = read(live_path)
    base, _ = read(base_path)
    base_fns = split_functions(base)

    # Collect every proposed replacement before touching anything, so a conflict
    # aborts the whole merge rather than leaving the file half-updated.
    proposals = {}   # name -> (agent, body lines)
    conflicts = []
    for path in agent_paths:
        who = os.path.basename(os.path.dirname(path)) or path
        cur, _ = read(path)
        for name, (s, e) in split_functions(cur).items():
            if name not in base_fns:
                print(f'  NEW      {who:14s} {name}  (not in baseline -- skipped)')
                continue
            bs, be = base_fns[name]
            if cur[s:e] == base[bs:be]:
                continue
            if name in proposals:
                conflicts.append((name, proposals[name][0], who))
                continue
            proposals[name] = (who, cur[s:e])

    if conflicts:
        print('CONFLICT -- two agents edited the same function. Nothing written.')
        for name, a, b in conflicts:
            print(f'  {name}: {a} and {b}')
        return 1

    if not proposals:
        print('no changes to merge')
        return 0

    # Apply against the LIVE file, bottom-up so earlier offsets stay valid.
    live_fns = split_functions(live)
    applied, missing = [], []
    for name, (who, body) in sorted(proposals.items(),
                                    key=lambda kv: -live_fns.get(kv[0], (0, 0))[0]):
        if name not in live_fns:
            missing.append((who, name))
            continue
        s, e = live_fns[name]
        live[s:e] = body
        applied.append((who, name, len(body)))
        live_fns = split_functions(live)

    for who, name, n in sorted(applied):
        print(f'  merged   {who:14s} {name}  ({n} lines)')
    for who, name in missing:
        print(f'  ABSENT   {who:14s} {name}  (gone from live draft -- skipped)')

    if dry:
        print(f'\n--dry-run: {len(applied)} function(s) would be merged, nothing written')
        return 0

    text = '\n'.join(live)
    with open(live_path, 'w', encoding='utf-8', newline='') as f:
        f.write(text.replace('\n', '\r\n') if live_crlf else text)
    print(f'\nwrote {live_path}: {len(applied)} function(s) merged')
    return 0


if __name__ == '__main__':
    sys.exit(main())
