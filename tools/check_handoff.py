#!/usr/bin/env python3
"""Consistency checker for NSMBW-Decomp's HANDOFF.md.

WHAT THIS IS FOR.  HANDOFF.md has repeatedly shipped with stale content that
misdirects the next session: a position figure from two sessions ago, a commit
count that moved, a "next target" section for a TU that was banked days ago.
This script catches that class of error mechanically.

    RUN IT BEFORE COMMITTING ANY HANDOFF UPDATE:
        python tools/check_handoff.py

Every fact it checks against is derived from the repository at run time --
progress.py's own arithmetic, git, slices/*.json, bin/dtk/*_symbols.txt -- so it
keeps working as the project moves.  It never writes to HANDOFF.md.

    python tools/check_handoff.py               # checks <repo>/HANDOFF.md
    python tools/check_handoff.py path/to/HANDOFF.md
    python tools/check_handoff.py --repo C:\\...\\NSMBW-Decomp some/copy.md
    python tools/check_handoff.py --self-test   # verify the checker itself

Exit code is non-zero only when a FAIL-class finding is present.  Heuristic
checks emit WARN so that the FAIL set stays trustworthy: a checker that cries
wolf is a checker people turn off.  When a check has to choose, it prefers
precision over recall.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Iterable, Optional

# --------------------------------------------------------------------------
# findings
# --------------------------------------------------------------------------

FAIL = 'FAIL'
WARN = 'WARN'


@dataclass
class Finding:
    level: str
    check: str
    line: int          # 1-based; 0 when not tied to a line
    message: str
    detail: str = ''

    def render(self) -> str:
        where = f'line {self.line}' if self.line else 'file'
        out = f'  {self.level} [{self.check}] {where}: {self.message}'
        if self.detail:
            out += f'\n        {self.detail}'
        return out


# --------------------------------------------------------------------------
# repository ground truth
# --------------------------------------------------------------------------

@dataclass
class TuExtent:
    start: int
    end: int
    span_bytes: int      # end - start, including inter-function padding
    code_bytes: int      # sum of function sizes inside the range
    fn_count: int
    source: str          # 'slice' (banked, exact) or 'sinit' (inferred)


@dataclass
class Facts:
    """Everything the checks compare HANDOFF.md against.

    Constructed from the repo by `load_facts`; constructed by hand in
    --self-test so the checks can be exercised without a fake repository.
    """
    # binary filename -> (compiled_bytes, total_bytes)
    per_binary: dict[str, tuple[int, int]] = field(default_factory=dict)
    project: tuple[int, int] = (0, 0)
    unpushed: Optional[int] = None
    banked: set[str] = field(default_factory=set)        # TU stems, matching
    non_matching: set[str] = field(default_factory=set)  # TU stems, nonMatching
    extents: dict[str, TuExtent] = field(default_factory=dict)
    repo_files: set[str] = field(default_factory=set)    # posix rel paths
    repo_dirs: set[str] = field(default_factory=set)
    basenames: set[str] = field(default_factory=set)     # every filename in repo
    notes: list[str] = field(default_factory=list)       # how facts were obtained

    def pct(self, key: str) -> Optional[float]:
        if key == 'TOTAL':
            c, t = self.project
        elif key in self.per_binary:
            c, t = self.per_binary[key]
        else:
            return None
        return (c / t) * 100 if t else None


CODE_SECTIONS = ('.init', '.text')          # same as progress.py
REPO_MARKERS = ('progress.py', 'slices', 'tools')


def find_repo(start: Path) -> Optional[Path]:
    for cand in [start, *start.parents]:
        if all((cand / m).exists() for m in REPO_MARKERS):
            return cand
    return None


def _load_progress_via_subprocess(repo: Path, facts: Facts) -> bool:
    """Invoke progress.py --progress-summary, the authority on position."""
    try:
        out = subprocess.run([sys.executable, 'progress.py', '--progress-summary'],
                             cwd=str(repo), capture_output=True, text=True, timeout=180)
    except Exception:
        return False
    if out.returncode != 0:
        return False
    pat = re.compile(r'^(\S+): Decompiled (\d+)/(\d+) code bytes')
    tot = re.compile(r'^Total: Decompiled (\d+)/(\d+) code bytes')
    ok = False
    for line in out.stdout.splitlines():
        line = line.strip()
        m = pat.match(line)
        # `pat` captures the name without its colon, so the grand total would
        # otherwise be registered as a binary called "Total".
        if m and m.group(1) != 'Total':
            facts.per_binary[m.group(1)] = (int(m.group(2)), int(m.group(3)))
            ok = True
        m = tot.match(line)
        if m:
            facts.project = (int(m.group(1)), int(m.group(2)))
    if ok:
        facts.notes.append('position: progress.py --progress-summary')
    return ok


def _load_progress_via_slicelib(repo: Path, facts: Facts) -> bool:
    """Fallback: reuse progress.py's own helper library directly."""
    sys.path.insert(0, str(repo / 'tools'))
    cwd = os.getcwd()
    try:
        os.chdir(repo)
        from slicelib import load_slice_file  # type: ignore
        tot_c = tot_t = 0
        for jf in sorted((repo / 'slices').glob('*.json')):
            sf = load_slice_file(jf)
            c = t = 0
            for sl in sf.parsed_slices:
                for sec in sl.sliceSecs:
                    if sec.sec_name not in CODE_SECTIONS:
                        continue
                    n = sec.end_offs - sec.start_offs
                    t += n
                    if sl.source and not sl.nonMatching:
                        c += n
            facts.per_binary[sf.meta.fileName] = (c, t)
            tot_c += c
            tot_t += t
        facts.project = (tot_c, tot_t)
        facts.notes.append('position: slicelib (progress.py fallback)')
        return bool(facts.per_binary)
    except Exception:
        return False
    finally:
        os.chdir(cwd)


def _load_slices_raw(repo: Path, facts: Facts) -> dict[str, list[dict]]:
    """Banked/nonMatching sources straight from the slice JSON."""
    raw: dict[str, list[dict]] = {}
    for jf in sorted((repo / 'slices').glob('*.json')):
        try:
            data = json.loads(jf.read_text(encoding='utf-8'))
        except Exception:
            continue
        raw[jf.name] = data.get('slices', [])
        for sl in data.get('slices', []):
            src = sl.get('source')
            if not src:
                continue
            stem = Path(src).stem
            if sl.get('nonMatching'):
                facts.non_matching.add(stem)
            else:
                facts.banked.add(stem)
    facts.notes.append(f'slices: {len(facts.banked)} banked, '
                       f'{len(facts.non_matching)} nonMatching')
    return raw


def _load_git(repo: Path, facts: Facts) -> None:
    def git(*a: str) -> Optional[str]:
        try:
            r = subprocess.run(['git', *a], cwd=str(repo), capture_output=True,
                               text=True, timeout=60)
            return r.stdout.strip() if r.returncode == 0 else None
        except Exception:
            return None
    n = git('rev-list', '--count', 'HEAD', '--not', '--remotes')
    if n is None:
        n = git('rev-list', '--count', '@{u}..HEAD')
    if n is not None and n.isdigit():
        facts.unpushed = int(n)
        facts.notes.append(f'git: {n} commits not on any remote')


_SYM_RE = re.compile(r'^([^\s=]+) = (\.\w+):(0x[0-9A-Fa-f]+); // type:(\w+)(?: size:(0x[0-9A-Fa-f]+))?')


def _load_extents(repo: Path, facts: Facts, raw_slices: dict[str, list[dict]]) -> None:
    """TU .text extents.

    Banked TUs get exact bounds from slices/wiimj2d.json.  Unbanked TUs are
    delimited by the `__sinit_\\<file>_cpp` chain -- the same derivation
    tools/tu_extent.py uses, reimplemented here so the checker does not depend
    on that script surviving.
    """
    sym = repo / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'
    slice_json = repo / 'slices' / 'wiimj2d.json'
    if not sym.exists() or not slice_json.exists():
        return
    try:
        meta = json.loads(slice_json.read_text(encoding='utf-8'))['meta']
        text_base = int(meta['sections']['.text']['addr'], 16)
    except Exception:
        return

    funcs: list[tuple[int, int]] = []
    sinits: list[tuple[int, str, int]] = []
    for line in sym.read_text(encoding='utf-8', errors='replace').splitlines():
        m = _SYM_RE.match(line.strip())
        if not m or m.group(2) != '.text' or m.group(4) != 'function':
            continue
        addr = int(m.group(3), 16)
        size = int(m.group(5), 16) if m.group(5) else 0
        funcs.append((addr, size))
        if m.group(1).startswith('__sinit_'):
            sinits.append((addr, m.group(1)[len('__sinit_'):].lstrip('\\'), size))
    funcs.sort()
    sinits.sort()
    if not funcs:
        return

    banked_ranges: list[tuple[int, int, str]] = []
    for sl in raw_slices.get('wiimj2d.json', []):
        rng = (sl.get('memoryRanges') or {}).get('.text')
        if not rng or not sl.get('source'):
            continue
        a, _, b = rng.partition('-')
        banked_ranges.append((text_base + int(a, 16), text_base + int(b, 16),
                              Path(sl['source']).stem))
    banked_ranges.sort()

    def window(lo: int, hi: int) -> tuple[int, int]:
        code = sum(s for a, s in funcs if lo <= a < hi)
        n = sum(1 for a, _ in funcs if lo <= a < hi)
        return code, n

    for lo, hi, stem in banked_ranges:
        code, n = window(lo, hi)
        facts.extents[stem] = TuExtent(lo, hi, hi - lo, code, n, 'slice')

    done = {stem for _, _, stem in banked_ranges}
    for i, (addr, name, size) in enumerate(sinits):
        stem = name[:-4] if name.endswith('_cpp') else name
        stem = stem.replace('_cpp', '')
        if stem in done:
            continue
        prev_end = sinits[i - 1][0] + sinits[i - 1][2] if i else text_base
        for lo, hi, _ in banked_ranges:          # overlap, not containment
            if lo < addr and hi > prev_end:
                prev_end = max(prev_end, hi)
        end = addr + size
        for fa, fs in funcs:                     # absorb trailing templates
            if fa == end:
                end = fa + fs
        nxt = sinits[i + 1][0] if i + 1 < len(sinits) else None
        if nxt and end > nxt:
            end = addr + size
        if end <= prev_end:
            continue
        code, n = window(prev_end, end)
        facts.extents[stem] = TuExtent(prev_end, end, end - prev_end, code, n, 'sinit')
    facts.notes.append(f'extents: {len(facts.extents)} TUs '
                       f'({len(done)} from slices, rest from __sinit chain)')


SKIP_DIRS = {'.git', 'build', '__pycache__', 'node_modules', '.ninja_deps'}


def _load_files(repo: Path, facts: Facts) -> None:
    for dirpath, dirnames, filenames in os.walk(repo):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        rel = Path(dirpath).relative_to(repo).as_posix()
        if rel != '.':
            facts.repo_dirs.add(rel)
        for f in filenames:
            p = f if rel == '.' else f'{rel}/{f}'
            facts.repo_files.add(p)
            facts.basenames.add(f)


def load_facts(repo: Path) -> Facts:
    facts = Facts()
    if not _load_progress_via_subprocess(repo, facts):
        _load_progress_via_slicelib(repo, facts)
    raw = _load_slices_raw(repo, facts)
    _load_git(repo, facts)
    _load_extents(repo, facts, raw)
    _load_files(repo, facts)
    return facts


# --------------------------------------------------------------------------
# document model
# --------------------------------------------------------------------------

@dataclass
class Heading:
    level: int
    text: str
    line: int      # 1-based
    end: int       # 1-based, exclusive: line of the next heading of level <= own


@dataclass
class Table:
    header: list[str]
    rows: list[tuple[int, list[str]]]   # (line no, cells)
    line: int
    heading: Optional[Heading] = None


@dataclass
class Doc:
    lines: list[str]
    prose: list[bool]          # False inside fenced code blocks
    headings: list[Heading]
    tables: list[Table]

    def heading_for(self, line: int) -> Optional[Heading]:
        best = None
        for h in self.headings:
            if h.line <= line < h.end:
                if best is None or h.level > best.level:
                    best = h
        return best


def parse_doc(text: str) -> Doc:
    lines = text.splitlines()
    prose = [True] * len(lines)
    fenced = False
    for i, ln in enumerate(lines):
        if ln.lstrip().startswith('```'):
            fenced = not fenced
            prose[i] = False
            continue
        prose[i] = not fenced

    headings: list[Heading] = []
    for i, ln in enumerate(lines):
        if not prose[i]:
            continue
        m = re.match(r'^(#{1,6})\s+(.*?)\s*$', ln)
        if m:
            headings.append(Heading(len(m.group(1)), m.group(2), i + 1, len(lines) + 1))
    for idx, h in enumerate(headings):
        for nxt in headings[idx + 1:]:
            if nxt.level <= h.level:
                h.end = nxt.line
                break

    tables: list[Table] = []
    i = 0
    while i < len(lines) - 1:
        if prose[i] and lines[i].lstrip().startswith('|') and \
           re.match(r'^\s*\|[\s:|-]+\|\s*$', lines[i + 1] or ''):
            header = _cells(lines[i])
            rows: list[tuple[int, list[str]]] = []
            j = i + 2
            while j < len(lines) and prose[j] and lines[j].lstrip().startswith('|'):
                rows.append((j + 1, _cells(lines[j])))
                j += 1
            t = Table(header, rows, i + 1)
            tables.append(t)
            i = j
        else:
            i += 1
    doc = Doc(lines, prose, headings, tables)
    for t in doc.tables:
        t.heading = doc.heading_for(t.line)
    return doc


def _cells(line: str) -> list[str]:
    s = line.strip()
    if s.startswith('|'):
        s = s[1:]
    if s.endswith('|'):
        s = s[:-1]
    return [c.strip() for c in s.split('|')]


def norm(s: str) -> str:
    """Lowercase, de-markdown, collapse whitespace.  Underscores are KEPT so
    that TU identifiers such as `d_a_en_blockmain` survive normalisation."""
    s = s.replace('`', '').replace('*', '')
    s = re.sub(r'[\u2010-\u2015]', '-', s)
    s = re.sub(r'[^\w\s.%/-]', ' ', s)
    return re.sub(r'\s+', ' ', s).strip().lower()


# --------------------------------------------------------------------------
# check 1: position figures
# --------------------------------------------------------------------------

PCT_RE = re.compile(r'(\d+(?:\.\d+)?)\s*%')
BYTEPAIR_RE = re.compile(r'(\d{1,3}(?:,\d{3})+|\d{5,})\s*/\s*(\d{1,3}(?:,\d{3})+|\d{5,})')
TRANSITION_RE = re.compile(r'^\s*(?:\*+\s*)?(?:->|-->|\u2192|\u21d2|to|up to|down to)\s*(?:\*+\s*)?$')
TRAIL_ARROW_RE = re.compile(r'^\s*(?:\*+\s*)?(?:->|-->|\u2192|\u21d2|to)\s*(?:\*+\s*)?$')
LEAD_ARROW_RE = re.compile(r'^\s*(?:\*+\s*)?(?:->|-->|\u2192|\u21d2)\s*(?:\*+\s*)?$')
POSITION_WORD_RE = re.compile(
    r'\b(position|progress|now at|currently at|currently|sits at|stands at|'
    r'we are at|project is at|total|overall)\b', re.I)

# "<A> B of the <B>-B total (45.4%)" -- a share of one size in another, which
# is a completely different quantity from a completion percentage.  Anchored at
# the end so it only matches when the ratio runs straight into the percentage;
# the filler may not contain a digit or a "%", which keeps it from reaching back
# across an earlier percentage on the same line.
_BIGNUM = r'(\d{1,3}(?:,\d{3})+|\d{3,})'
SHARE_RE = re.compile(
    _BIGNUM + r'\s*(?:-\s*)?(?:B\b|bytes?\b)?\s*(?:\*+)?\s*\bof\b\s*(?:the\s+)?'
    + _BIGNUM + r'\s*(?:-\s*)?(?:B\b|bytes?\b)?[^%\d]{0,30}$',
    re.I | re.S)
# A percentage that is immediately called "done"/"decompiled" is a completion
# figure even when no position word precedes it.
COMPLETION_AFTER_RE = re.compile(
    r'^\s*(?:\*+\s*)?[,—-]?\s*'
    r'(?:done|complete|completed|decompiled|matched|matching)\b', re.I)


def _num(s: str) -> int:
    return int(s.replace(',', ''))


def _fmt_like(value: float, claim: str) -> str:
    dec = len(claim.split('.')[1]) if '.' in claim else 0
    return f'{value:.{dec}f}'


def _binary_aliases(facts: Facts) -> dict[str, str]:
    """How the prose refers to each binary: `wiimj2d.dol`, `wiimj2d`, "the DOL".

    A bare extension is only an alias when exactly one binary carries it, so
    "the REL" stays ambiguous (and therefore unmatched) while "the DOL" does not.
    """
    aliases: dict[str, str] = {}
    by_ext: dict[str, list[str]] = {}
    for name in facts.per_binary:
        aliases[name.lower()] = name
        stem, _, ext = name.rpartition('.')
        if stem:
            aliases[stem.lower()] = name
            by_ext.setdefault(ext.lower(), []).append(name)
    for ext, names in by_ext.items():
        if len(names) == 1:
            aliases.setdefault(ext, names[0])
    return aliases


def _subject_binary(ctx: str, aliases: dict[str, str]) -> Optional[str]:
    """The binary that a bare completion figure ("... is 20.968% done") is about:
    the most recently named one in the text leading up to it."""
    low = ctx.lower()
    best_pos, best = -1, None
    for alias, name in aliases.items():
        for m in re.finditer(r'(?<![\w.])' + re.escape(alias) + r'(?![\w])', low):
            if m.start() > best_pos:
                best_pos, best = m.start(), name
    return best


def check_position(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []
    if not facts.project[1]:
        return out
    proj_c, proj_t = facts.project
    denominators = {proj_t: 'TOTAL'}
    for name, (c, t) in facts.per_binary.items():
        denominators.setdefault(t, name)
    aliases = _binary_aliases(facts)

    for i, raw in enumerate(doc.lines):
        if not doc.prose[i]:
            continue
        lineno = i + 1

        # -- byte pairs: N / M --------------------------------------------
        for m in BYTEPAIR_RE.finditer(raw):
            num, den = _num(m.group(1)), _num(m.group(2))
            key = denominators.get(den)
            if key is None:
                if proj_t and abs(den - proj_t) / proj_t < 0.02:
                    out.append(Finding(
                        FAIL, 'POSITION', lineno,
                        f'byte total {m.group(2)} is not the project total',
                        f'progress.py total is {proj_t:,} bytes'))
                continue
            want = proj_c if key == 'TOTAL' else facts.per_binary[key][0]
            if num != want:
                label = 'project' if key == 'TOTAL' else key
                out.append(Finding(
                    FAIL, 'POSITION', lineno,
                    f'{label} decompiled bytes claimed {m.group(1)}, actual {want:,}',
                    f'in "{m.group(0)}"'))

        # -- percentages ---------------------------------------------------
        pcts = list(PCT_RE.finditer(raw))
        if not pcts:
            continue
        historical = set()
        for a, b in zip(pcts, pcts[1:]):
            if TRANSITION_RE.match(raw[a.end():b.start()]):
                historical.add(a.start())
                historical.add(b.start())
        # A transition can straddle a line break ("18.891% ->" / "**19.200%**").
        if TRAIL_ARROW_RE.search(raw[pcts[-1].end():]):
            historical.add(pcts[-1].start())
        if LEAD_ARROW_RE.match(raw[:pcts[0].start()]):
            historical.add(pcts[0].start())
        # A sentence can straddle a line break ("...2,950,464 B** of the" /
        # "6,500,368-B total (45.4%)..."), so the context reaches one prose
        # line back.
        prev_line = doc.lines[i - 1] if i and doc.prose[i - 1] else ''
        for idx, m in enumerate(pcts):
            if m.start() in historical:
                continue
            prev_end = pcts[idx - 1].end() if idx else 0
            nxt_start = pcts[idx + 1].start() if idx + 1 < len(pcts) else len(raw)
            before = raw[prev_end:m.start()]
            window = raw[prev_end:nxt_start]
            ctx = (prev_line + '\n' if prev_line else '') + raw[:m.start()]
            claim = m.group(1)

            key = None
            for name in sorted(facts.per_binary, key=len, reverse=True):
                if name in before:
                    key = name
                    break
            if key is None:
                for bm in BYTEPAIR_RE.finditer(window):
                    if _num(bm.group(2)) == proj_t:
                        key = 'TOTAL'
                        break

            # -- share, not position: "A of B (X%)" ------------------------
            # Dismiss it only when A/B really is X.  A share whose own
            # arithmetic is wrong is still a defect and still gets reported.
            if key is None:
                sm = SHARE_RE.search(ctx)
                if sm:
                    a, b = _num(sm.group(1)), _num(sm.group(2))
                    if b:
                        got = (a / b) * 100
                        if _fmt_like(got, claim) == claim:
                            continue
                        out.append(Finding(
                            FAIL, 'POSITION', lineno,
                            f'share claimed {claim}%, but {a:,} of {b:,} '
                            f'is {got:.3f}%',
                            f'in "{raw.strip()[:110]}"'))
                        continue

            if key is None and POSITION_WORD_RE.search(before):
                key = 'TOTAL'
            # "<subject> ... is 20.968% done": the trailing word makes it a
            # completion figure, and the subject says whose.
            if key is None and COMPLETION_AFTER_RE.match(raw[m.end():]):
                key = _subject_binary(ctx, aliases)
            if key is None:
                continue                      # not a position claim; stay quiet
            actual = facts.pct(key)
            if actual is None:
                continue
            if _fmt_like(actual, claim) != claim:
                label = 'project total' if key == 'TOTAL' else key
                out.append(Finding(
                    FAIL, 'POSITION', lineno,
                    f'{label} claimed {claim}%, actual {actual:.3f}%',
                    f'in "{raw.strip()[:110]}"'))
    return out


# --------------------------------------------------------------------------
# check 2: unpushed commits
# --------------------------------------------------------------------------

UNPUSHED_CTX_RE = re.compile(r'unpushed|not (?:been )?pushed|ahead of (?:`?origin|the remote)', re.I)


def check_unpushed(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []
    if facts.unpushed is None:
        return out
    for i, raw in enumerate(doc.lines):
        if not doc.prose[i] or not UNPUSHED_CTX_RE.search(raw):
            continue
        for m in re.finditer(r'(\d+)\s*(?:\*+\s*)?(commits?)\b', raw, re.I):
            n = int(m.group(1))
            if n != facts.unpushed:
                out.append(Finding(
                    FAIL, 'UNPUSHED', i + 1,
                    f'claims {n} unpushed commit(s); git says {facts.unpushed}',
                    f'in "{raw.strip()[:110]}"'))
    return out


# --------------------------------------------------------------------------
# check 3: finished TU described as pending  (the loudest check)
# --------------------------------------------------------------------------

PENDING_RE = re.compile(
    r'\bNEXT UP\b|\bnext up\b|\bnext target\b|\bnext targets\b|\bthe next TU\b|'
    r'\bis next\b|\bshould be next\b|\bstart (?:with|on|here)\b|\bTODO\b|\bto do\b|'
    r'\bnot started\b|\bunstarted\b|\bnot yet (?:started|begun|done)\b|'
    r'\bwork plan\b|\bpick (?:this|it) up\b|\bpending\b|\byet to be\b|'
    r'\bremains? to be (?:done|written|authored)\b|\bawaiting\b', re.I)
DONE_RE = re.compile(
    r'\bDONE\b|\bdone\b|\blanded\b|\bbanked\b|\bbyte-exact\b|\bcomplete[d]?\b|'
    r'\bfinished\b|\bmatching\b|\bshipped\b|\bverif(?:ied|ying)\b|\bclosed\b', re.I)
PENDING_TABLE_HEADING_RE = re.compile(r'remaining|next target|candidate|to do|todo|backlog', re.I)


CODE_EXT_RE = re.compile(r'^\.(?:cpp|cxx|cc|c|hpp|h|o|s|a|inc)(?![\w])')
BACKTICK_SPAN_RE = re.compile(r'`[^`]*`')


def _is_distinctive(stem: str) -> bool:
    """Whether a bare prose occurrence of this TU basename means the TU.

    Several banked TUs have basenames that are also ordinary English words or
    are simply too short to be unambiguous -- `list` (lib/nw4hbm/ut/list.cpp),
    `mdl`, `banm`, `d_res`, `d_next`, `c_tree`, `m_3d` ... For those, "the SDK
    list further down" is not a mention of a translation unit, and treating it
    as one is how this check would end up being ignored.

    A long underscored identifier such as `d_a_en_blockmain` cannot appear by
    accident, so it still counts bare -- which matters, because that is exactly
    the shape of the failure this check exists to catch.
    """
    return len(stem) >= 8 and '_' in stem


def _tu_mentions(line: str, stems: Iterable[str]) -> list[str]:
    """TU basenames named on this line, written as code rather than as prose.

    A name counts when it is backticked, carries a source/object extension, or
    sits in a path -- or when it is distinctive enough to stand alone.
    """
    spans = [(m.start(), m.end()) for m in BACKTICK_SPAN_RE.finditer(line)]
    hits = []
    for stem in stems:
        distinctive = _is_distinctive(stem)
        for m in re.finditer(r'(?<![\w.])' + re.escape(stem) + r'(?![\w])', line):
            tail = line[m.end():]
            if (distinctive
                    or any(a < m.start() and m.end() <= b for a, b in spans)
                    or CODE_EXT_RE.match(tail)
                    or tail.startswith('/')
                    or line[:m.start()].endswith('/')):
                hits.append(stem)
                break
    return hits


def check_banked_pending(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []
    if not facts.banked:
        return out
    # Longest stems first so `d_a_en_kuribo_base` never matches as `d_a_en_kuribo`.
    stems = sorted(facts.banked, key=len, reverse=True)

    for i, raw in enumerate(doc.lines):
        if not doc.prose[i]:
            continue
        pend = PENDING_RE.search(raw)
        if not pend:
            continue
        hits = _tu_mentions(raw, stems)
        # A stem matched inside a longer banked stem is not a separate mention.
        hits = [h for h in hits if not any(h != o and h in o for o in hits)]
        if not hits:
            continue
        lineno = i + 1
        is_heading = bool(re.match(r'^#{1,6}\s', raw))
        done = DONE_RE.search(raw)
        for stem in hits:
            tu_pos = raw.find(stem)
            # "<TU> ... NEXT UP ... banked" reads as a claim about this TU even
            # though a done-word appears later (it is describing a neighbour).
            # "<TU> ... DONE ... the next target is ..." does not.
            claims_this_tu = tu_pos != -1 and tu_pos < pend.start() and \
                (done is None or pend.start() < done.start())
            if done and not claims_this_tu:
                out.append(Finding(
                    WARN, 'BANKED-PENDING', lineno,
                    f'`{stem}` is banked and matching, and this line mixes a '
                    f'pending marker ("{pend.group(0)}") with a done marker',
                    f'in "{raw.strip()[:110]}"'))
            else:
                out.append(Finding(
                    FAIL, 'BANKED-PENDING', lineno,
                    f'`{stem}` is BANKED AND MATCHING in slices/, but this '
                    f'{"heading" if is_heading else "line"} calls it pending '
                    f'("{pend.group(0)}")',
                    f'in "{raw.strip()[:110]}"'))

    # Rows of a "remaining / next targets" table that name a banked TU.
    for t in doc.tables:
        head = t.heading.text if t.heading else ''
        if not PENDING_TABLE_HEADING_RE.search(head):
            continue
        for lineno, cells in t.rows:
            if not cells:
                continue
            row = ' | '.join(cells)
            if PENDING_RE.search(row):
                continue                        # already reported above as FAIL
            hits = _tu_mentions(cells[0], stems)
            hits = [h for h in hits if not any(h != o and h in o for o in hits)]
            if hits and not DONE_RE.search(row):
                out.append(Finding(
                    WARN, 'BANKED-PENDING', lineno,
                    f'`{hits[0]}` is banked and matching but is listed, with no '
                    f'done marker, under "{head.strip()}"',
                    f'in "{row[:110]}"'))

    # Section whose heading names a banked TU and whose body is a numbered plan.
    for h in doc.headings:
        hits = _tu_mentions(h.text, stems)
        hits = [x for x in hits if not any(x != o and x in o for o in hits)]
        if not hits or DONE_RE.search(h.text):
            continue
        body = doc.lines[h.line:min(h.end - 1, len(doc.lines))]
        steps = sum(1 for b in body if re.match(r'^\s*(\d+\.|- \[ \])\s', b))
        if steps >= 3:
            out.append(Finding(
                WARN, 'BANKED-PENDING', h.line,
                f'section "{h.text.strip()[:70]}" names banked TU `{hits[0]}` and '
                f'contains a {steps}-step plan with no done marker in the heading',
                'a work plan for a finished TU is the failure this file has hit repeatedly'))
    return out


# --------------------------------------------------------------------------
# check 4: duplicate headings
# --------------------------------------------------------------------------

def check_duplicate_headings(doc: Doc, facts: Facts) -> list[Finding]:
    seen: dict[str, list[Heading]] = {}
    for h in doc.headings:
        seen.setdefault(norm(h.text).rstrip('.:'), []).append(h)
    out: list[Finding] = []
    for key, hs in seen.items():
        if len(hs) < 2 or not key:
            continue
        first = hs[0]
        for dup in hs[1:]:
            out.append(Finding(
                FAIL, 'DUP-HEADING', dup.line,
                f'heading "{dup.text.strip()[:80]}" duplicates line {first.line}',
                'a whole section was duplicated verbatim once before'))
    return out


# --------------------------------------------------------------------------
# check 5: target table accuracy
# --------------------------------------------------------------------------

TU_COL_RE = re.compile(r'^(tu|translation unit|file|target)$', re.I)
BYTES_COL_RE = re.compile(r'^(bytes?|size|\.text bytes?)$', re.I)
FNS_COL_RE = re.compile(r'^(fns?|functions?|func|funcs)$', re.I)


def check_target_table(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []
    if not facts.extents:
        return out
    for t in doc.tables:
        cols = {}
        for idx, c in enumerate(t.header):
            n = norm(c)
            if TU_COL_RE.match(n):
                cols.setdefault('tu', idx)
            elif BYTES_COL_RE.match(n):
                cols.setdefault('bytes', idx)
            elif FNS_COL_RE.match(n):
                cols.setdefault('fns', idx)
        if 'tu' not in cols or 'bytes' not in cols:
            continue
        for lineno, cells in t.rows:
            if len(cells) <= max(cols.values()):
                continue
            name = norm(cells[cols['tu']]).replace('.cpp', '').strip()
            ext = facts.extents.get(name)
            if ext is None:
                continue
            if ext.source == 'slice':
                # Already-banked TU: its row is a historical record of the
                # pre-landing estimate, and the real bounds have since moved.
                # Checking it here only manufactures noise -- a banked TU that
                # is still being presented as work to do is BANKED-PENDING's job.
                continue
            bm = re.search(r'(\d[\d,]*)', cells[cols['bytes']])
            if not bm:
                continue
            claimed = _num(bm.group(1))
            interp = None
            if claimed == ext.span_bytes:
                interp = 'span'
            elif claimed == ext.code_bytes:
                interp = 'code bytes'
            src = 'exact, from slices' if ext.source == 'slice' else 'inferred from __sinit chain'
            if interp is None:
                out.append(Finding(
                    WARN, 'TARGET-TABLE', lineno,
                    f'`{name}` byte size {claimed:,} matches neither span '
                    f'({ext.span_bytes:,}) nor code bytes ({ext.code_bytes:,})',
                    f'range {ext.start:#x}-{ext.end:#x} ({src})'))
            if 'fns' in cols and len(cells) > cols['fns']:
                fm = re.search(r'(\d[\d,]*)', cells[cols['fns']])
                if fm and _num(fm.group(1)) != ext.fn_count:
                    out.append(Finding(
                        WARN, 'TARGET-TABLE', lineno,
                        f'`{name}` function count {_num(fm.group(1))} disagrees '
                        f'with {ext.fn_count} symbols in {ext.start:#x}-{ext.end:#x}',
                        f'byte size matched by {interp or "no"} interpretation ({src})'))
    return out


# --------------------------------------------------------------------------
# check 6: internal cross-references
# --------------------------------------------------------------------------

MISSING_CTX_RE = re.compile(
    r'\blost\b|no longer exist|does not exist|doesn.t exist|never existed|'
    r'\bdeleted\b|\bremoved\b|\babsent\b|\bgone\b|not tracked|gitignored|'
    r'\bis stale\b|\bmissing\b|\bpredates\b|\bwas lost\b', re.I)
PATHY_RE = re.compile(r'`([A-Za-z0-9_./\\-]+\.(?:py|md|json|txt|sh|html|yaml|yml|cfg|ini|lcf))`')
SECREF_RE = re.compile(
    r'(?:see|See|described|recorded|documented|listed|covered)\s+'
    r'(?:the\s+)?(?:section\s+)?(?:in|under|at|below|above)?\s*'
    r'["\u201c]([^"\u201d]{4,90})["\u201d]', re.U)
# Skip source-tree-shaped prefixes that are not repo-relative paths.
SKIP_PREFIXES = ('scratchpad/', 'work/', 'dol/', 'rel/', 'runtime/', 'lib/', 'http', 'build/')


def check_xrefs(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []
    if not facts.repo_files:
        return out
    top_dirs = {d.split('/')[0] for d in facts.repo_dirs}

    # Names the document itself declares gone anywhere -> never re-flag them.
    declared_gone: set[str] = set()
    for i, raw in enumerate(doc.lines):
        if doc.prose[i] and MISSING_CTX_RE.search(raw):
            for m in PATHY_RE.finditer(raw):
                declared_gone.add(Path(m.group(1)).name)

    for i, raw in enumerate(doc.lines):
        if not doc.prose[i]:
            continue
        lineno = i + 1
        negated = bool(MISSING_CTX_RE.search(raw))
        for m in PATHY_RE.finditer(raw):
            ref = m.group(1).replace('\\', '/')
            base = Path(ref).name
            if base in declared_gone or ref.startswith(SKIP_PREFIXES):
                continue
            if '/' in ref:
                if ref.split('/')[0] not in top_dirs:
                    continue                      # not a repo-relative path
                if ref in facts.repo_files or ref in facts.repo_dirs:
                    continue
                if negated:
                    continue
                out.append(Finding(
                    FAIL, 'XREF-FILE', lineno,
                    f'`{ref}` does not exist in the repository',
                    f'in "{raw.strip()[:110]}"'))
            else:
                if not ref.endswith('.py') or ref in facts.basenames or negated:
                    continue
                out.append(Finding(
                    WARN, 'XREF-FILE', lineno,
                    f'`{ref}` is referenced but no file of that name exists anywhere',
                    f'in "{raw.strip()[:110]}"'))

    heads = [(norm(h.text), h) for h in doc.headings]
    for i, raw in enumerate(doc.lines):
        if not doc.prose[i]:
            continue
        for m in SECREF_RE.finditer(raw):
            ref = norm(m.group(1))
            if not ref or len(ref.split()) < 2:
                continue
            if any(ref in ht or ht in ref for ht, _ in heads):
                continue
            rt = set(ref.split())
            if any(len(rt & set(ht.split())) / max(1, len(rt)) >= 0.7 for ht, _ in heads):
                continue
            out.append(Finding(
                WARN, 'XREF-SECTION', i + 1,
                f'reference to a section "{m.group(1)[:70]}" that no heading matches',
                f'in "{raw.strip()[:110]}"'))
    return out


# --------------------------------------------------------------------------
# check 7: stale count claims about tools / defects
# --------------------------------------------------------------------------

CARDINALS = {'both': 2, 'twice': 2, 'thrice': 3, 'two': 2, 'three': 3, 'four': 4,
             'five': 5, 'six': 6, 'seven': 7, 'eight': 8, 'nine': 9, 'ten': 10}
ORDINAL_LEAD_RE = re.compile(r'^\s*(?:the|a|an)\s+(second|third|fourth|fifth|sixth|seventh)\b', re.I)
DEFECT_NOUN = (r'incidents?|bugs?|defects?|failures?|lies|caveats?|occasions?|'
               r'mistakes?|errors?|regressions?|times')
DEFECT_VERB_RE = re.compile(r'lied|lie\b|lying|wrong|failed|fail\b|broken|misreport|silently', re.I)
CLAIM_A_RE = re.compile(r'\b(twice|thrice)\b', re.I)
CLAIM_B_RE = re.compile(r'\b(both|two|three|four|five|six|seven|eight|nine|ten|\d+)\s+'
                        r'(?:\w+\s+){0,2}(' + DEFECT_NOUN + r')\b', re.I)
SUBCOUNT_NOUN_RE = re.compile(
    r'\b(both|two|three|four|five|six|seven|eight|nine|ten|\d+)\s+(?:\w+\s+){0,2}'
    r'(rules?|lessons?|caveats?|parts?|reasons?|steps?|checks?|cases?|levers?|'
    r'incidents?|bugs?|defects?|techniques?|gotchas?)\b', re.I)


def _card(tok: str) -> Optional[int]:
    tok = tok.lower()
    if tok.isdigit():
        return int(tok)
    return CARDINALS.get(tok)


def _split_topic(text: str) -> Optional[str]:
    for sep in ('\u2014', '\u2013', ' -- ', ':'):
        if sep in text:
            prefix = text.split(sep)[0]
            n = norm(prefix)
            if len(n.split()) >= 3:
                return n
    return None


def check_stale_counts(doc: Doc, facts: Facts) -> list[Finding]:
    out: list[Finding] = []

    # -- 7a: an ordinal chain of headings vs a cardinal claim about it -----
    chains: dict[str, list[Heading]] = {}
    for h in doc.headings:
        topic = _split_topic(h.text)
        if topic:
            chains.setdefault(topic, []).append(h)
    for topic, hs in chains.items():
        if len(hs) < 2:
            continue
        documented = len(hs)
        last = hs[-1]
        extra = []
        for h in doc.headings:
            if h.line > last.line and h.level == last.level and ORDINAL_LEAD_RE.match(h.text):
                if all(o.line < h.line for o in hs):
                    documented += 1
                    extra.append(h)
                    last = h
        topic_words = set(topic.split())
        spans = [(h.line, h.end) for h in hs + extra]
        offenders: list[tuple[int, int, str]] = []
        for i, raw in enumerate(doc.lines):
            if not doc.prose[i]:
                continue
            lineno = i + 1
            in_topic = any(a <= lineno < b for a, b in spans)
            n = norm(raw)
            relevant = in_topic or topic in n or \
                len(topic_words & set(n.split())) >= max(2, len(topic_words) - 1)
            if not relevant:
                continue
            claim = None
            m = CLAIM_A_RE.search(raw)
            if m and DEFECT_VERB_RE.search(raw[max(0, m.start() - 60):m.end() + 60]):
                claim = (_card(m.group(1)), m.group(0))
            if claim is None:
                m = CLAIM_B_RE.search(raw)
                if m:
                    claim = (_card(m.group(1)), m.group(0))
            if claim and claim[0] is not None and claim[0] < documented:
                offenders.append((lineno, claim[0], claim[1]))
        if offenders:
            names = ', '.join(f'line {h.line}' for h in hs + extra)
            for lineno, val, txt in offenders:
                out.append(Finding(
                    FAIL, 'STALE-COUNT', lineno,
                    f'claims {val} ("{txt}") but the file documents {documented} '
                    f'instances of "{topic}"',
                    f'instances at {names}'))

    # -- 7b: "N rules/lessons/parts" vs the sub-headings that follow ------
    for idx, h in enumerate(doc.headings):
        children = [c for c in doc.headings
                    if h.line < c.line < h.end and c.level == h.level + 1]
        if len(children) < 2:
            continue
        # Only two places genuinely enumerate the sub-sections: the heading
        # itself, and a lead-in line that ends with a colon.  Counting claims
        # from ordinary prose ("fixed two tool bugs") is how this check would
        # start crying wolf.
        candidates = [(h.line, h.text)]
        for ln in range(h.line, min(h.line + 10, h.end - 1)):
            if ln - 1 < len(doc.lines):
                body = doc.lines[ln - 1].strip()
                if body.rstrip('*_ ').endswith(':'):
                    candidates.append((ln, body))
        for src_line, txt in candidates:
            m = SUBCOUNT_NOUN_RE.search(txt)
            if not m:
                continue
            val = _card(m.group(1))
            if val is not None and val < len(children):
                out.append(Finding(
                    FAIL, 'STALE-COUNT', src_line,
                    f'"{h.text.strip()[:60]}" claims {val} ({m.group(0)}) but has '
                    f'{len(children)} sub-sections',
                    'sub-sections at lines ' + ', '.join(str(c.line) for c in children)))
                break
    return out


# --------------------------------------------------------------------------
# driver
# --------------------------------------------------------------------------

CHECKS: list[tuple[str, str, Callable[[Doc, Facts], list[Finding]]]] = [
    ('POSITION', 'position figures vs progress.py', check_position),
    ('UNPUSHED', 'unpushed commit count vs git', check_unpushed),
    ('BANKED-PENDING', 'banked TUs described as pending', check_banked_pending),
    ('DUP-HEADING', 'duplicate headings', check_duplicate_headings),
    ('TARGET-TABLE', 'target table vs symbol map', check_target_table),
    ('XREF-FILE', 'cross-references to files', check_xrefs),
    ('STALE-COUNT', 'stale defect/count claims', check_stale_counts),
]

# check_xrefs emits two codes; this one has no function of its own.
DESCRIPTIONS = {name: desc for name, desc, _ in CHECKS}
DESCRIPTIONS['XREF-SECTION'] = 'cross-references to sections'
CHECK_ORDER = [name for name, _, _ in CHECKS]
CHECK_ORDER.insert(CHECK_ORDER.index('XREF-FILE') + 1, 'XREF-SECTION')


def run_checks(doc: Doc, facts: Facts) -> list[Finding]:
    findings: list[Finding] = []
    for _, _, fn in CHECKS:
        findings.extend(fn(doc, facts))
    return findings


def report(findings: list[Finding], facts: Facts, path: Path, verbose: bool) -> int:
    order = {name: i for i, name in enumerate(CHECK_ORDER)}
    findings = sorted(findings, key=lambda f: (order.get(f.check, 99),
                                               0 if f.level == FAIL else 1, f.line))
    fails = [f for f in findings if f.level == FAIL]
    warns = [f for f in findings if f.level == WARN]

    print(f'check_handoff: {path}')
    if verbose:
        for n in facts.notes:
            print(f'  . {n}')
    if not findings:
        print('  no findings.')
    last = None
    for f in findings:
        if f.check != last:
            print(f'\n-- {f.check}: {DESCRIPTIONS.get(f.check, "")}')
            last = f.check
        print(f.render())
    print()
    print(f'SUMMARY: {len(fails)} FAIL, {len(warns)} WARN '
          f'across {len(CHECK_ORDER)} checks -> exit {1 if fails else 0}')
    return 1 if fails else 0


# --------------------------------------------------------------------------
# self-test
# --------------------------------------------------------------------------

def _synthetic_facts() -> Facts:
    f = Facts()
    # Realistic magnitudes: the byte-pair rule deliberately ignores small
    # numbers ("46/53 words match"), so the fixture must be project-scale.
    f.per_binary = {'wiimj2d.dol': (640496, 3054592),        # 20.968%
                    'd_x.rel': (112, 356396),                # 0.031%
                    'd_y.rel': (0, 128000)}                  # keeps "REL"
    f.project = (692728, 6500368)                            # 10.657%
    f.unpushed = 7
    # `list`, `mdl` and `d_res` are real banked TUs whose basenames collide with
    # ordinary prose; they are here so the control can prove they stay quiet.
    f.banked = {'d_a_en_blockmain', 'd_a_thing', 'list', 'mdl', 'd_res'}
    f.extents = {
        'd_a_en_blockmain': TuExtent(0x100, 0x200, 0x100, 0xF0, 90, 'slice'),
        'd_a_ghost': TuExtent(0x300, 0x400, 0x100, 0xF0, 12, 'sinit'),
    }
    f.repo_files = {'tools/real_tool.py', 'progress.py'}
    f.repo_dirs = {'tools', 'source'}
    f.basenames = {'real_tool.py', 'progress.py'}
    return f


# (label, expected check code, minimum findings, minimum FAIL-level findings,
#  synthetic document)
SELF_TEST_CASES: list[tuple[str, str, int, int, str]] = [
    ('POSITION', 'POSITION', 4, 0, """
# Doc
- **Position: 10.453%** (679,496 / 6,500,368 code bytes); `wiimj2d.dol` at **20.535%**.
Historical: 8.475% -> 9.100% is fine and must not fire.
Coverage of 100% headers is fine and must not fire.
The DOL holds **2,950,464 B** of the 6,500,368-B total (51.2%) and is 20.968% done.
"""),
    ('POSITION-DONE', 'POSITION', 1, 1, """
# Doc
The DOL holds **2,950,464 B** of the 6,500,368-B total (45.4%) and is 21.400% done.
"""),
    ('UNPUSHED', 'UNPUSHED', 1, 0, """
# Doc
- **27 commits are unpushed.** Nothing has been pushed this week.
"""),
    ('BANKED-PENDING', 'BANKED-PENDING', 2, 0, """
# Doc
## Remaining targets
- **Next target: `d_a_en_blockmain.cpp`** (256 B, 90 fns).

| TU | Bytes | Fns | Notes |
|---|---|---|---|
| `d_a_en_blockmain` | 256 | 90 | **NEXT UP.** Sits below bros, whose banked bound it inherits |
| `d_a_ghost` | 256 | 12 | |

`d_a_thing` is DONE and landed; the next target after it is something else.
"""),
    # The failure this file has actually hit, written without backticks: a
    # banked TU presented as the next thing to do.  Stricter TU-name matching
    # must not let this through.
    ('BANKED-PENDING-BARE', 'BANKED-PENDING', 2, 2, """
# Doc
## Remaining targets
Next target: d_a_en_blockmain — 256 B across 90 functions, and it is unstarted.

| TU | Bytes | Fns | Notes |
|---|---|---|---|
| d_a_en_blockmain | 256 | 90 | **NEXT UP.** inherits the banked bound above it |
"""),
    ('DUP-HEADING', 'DUP-HEADING', 1, 0, """
# Doc
## Monitoring agents — what actually works
text
## Monitoring agents — what actually works
text
"""),
    ('TARGET-TABLE', 'TARGET-TABLE', 2, 0, """
# Doc
## Candidates
| TU | Bytes | Fns |
|---|---|---|
| `d_a_ghost` | 999 | 41 |
"""),
    ('XREF-FILE', 'XREF-FILE', 1, 0, """
# Doc
Run `tools/does_not_exist.py` to regenerate. Also `tools/real_tool.py` is fine.
`scratchpad/gone.py` must not fire, and a `lost_tool.py` that was **lost** must not fire.
"""),
    ('XREF-SECTION', 'XREF-SECTION', 1, 0, """
# Doc
## A heading that does exist
See "A Section That Does Not Exist Anywhere" for details.
See "A heading that does exist" — this one must not fire.
"""),
    # Exercises both sub-rules: the ordinal chain (7a) and the
    # "N rules" vs sub-section count (7b).
    ('STALE-COUNT', 'STALE-COUNT', 2, 0, """
# Doc
## Verify your verification tool — the comparator has lied twice
body
### Verify your verification tool — again
body
### Verify your verification tool — a third time
body
### The fourth view: it is blind to order
body

## The pakkun pair — DONE, and the two rules it cost
It fixed two tool bugs in passing, which must not be counted as a rule.
### Rule one
body
### Rule two
body
### Rule three
body
"""),
]


def self_test() -> int:
    facts = _synthetic_facts()
    print('check_handoff --self-test')
    print('Each case is a synthetic document planted with a known defect.\n')
    results: list[tuple[str, bool, int, int, int, int, list[str]]] = []
    for label, expect, need, need_fail, text in SELF_TEST_CASES:
        doc = parse_doc(text)
        found = run_checks(doc, facts)
        fired = [f for f in found if f.check == expect]
        nfail = sum(1 for f in fired if f.level == FAIL)
        others = sorted({f.check for f in found if f.check != expect})
        ok = len(fired) >= need and nfail >= need_fail
        results.append((label, ok, len(fired), need, nfail, need_fail, others))

    width = max(len(r[0]) for r in results)
    for label, ok, n, need, nfail, need_fail, others in results:
        flag = 'FIRED  ' if ok else 'MISSED '
        extra = f'   (also fired: {", ".join(others)})' if others else ''
        lvl = f', {nfail}/{need_fail} at FAIL' if need_fail else ''
        print(f'  {flag} {label:<{width}}  {n}/{need} expected finding(s){lvl}{extra}')

    # negative control: a clean document must produce nothing at all
    clean = """
# Doc
- **Position: 10.657%** (692,728 / 6,500,368 code bytes); `wiimj2d.dol` at **20.968%**.
- **7 commits are unpushed.** The residual is 46/53 words, which is not a position claim.
`tools/real_tool.py` works. Historical: 8.475% -> 9.100%.
A transition split across a line break: `wiimj2d.dol` 18.891% ->
**19.200%** must not fire, nor must 100% header coverage or 0.3-2.3% ranges.
See "Remaining targets" below. Three separate agents hit this bug.
It fixed two tool bugs in passing, which is prose, not an enumeration.
```
## Monitoring agents
## Monitoring agents
```
The DOL holds **2,950,464 B** of the
6,500,368-B total (45.4%) and is 20.968% done, so most of the undone work is
DOL game code — roughly 40% of everything remaining.
This is the only "next target" section for game code. The SDK list further down
is deprioritised, the mdl handling is not a next target either, and neither is
the d_res lookup path.
## Remaining targets
| TU | Bytes | Fns | Notes |
|---|---|---|---|
| `d_a_ghost` | 256 | 12 | |
| `d_a_en_blockmain` | 256 | 90 | **DONE**, landed and linked |

## The SDK list, ranked by expected cost
1. one
2. two
3. three
4. four
5. five
"""
    clean_findings = run_checks(parse_doc(clean), facts)
    clean_ok = not clean_findings
    print(f'\n  {"PASS   " if clean_ok else "FAIL   "} negative control '
          f'(clean document produces no findings): {len(clean_findings)} finding(s)')
    for f in clean_findings:
        print('        unexpected:', f.render().strip())

    missed = [r[0] for r in results if not r[1]]
    covered = {expect for _, expect, _, _, _ in SELF_TEST_CASES}
    print()
    if not missed and clean_ok:
        print(f'SELF-TEST PASSED: all {len(covered)} checks fired across '
              f'{len(results)} synthetic inputs, and the clean control produced '
              f'no findings.')
        return 0
    if missed:
        print(f'SELF-TEST FAILED: {len(missed)} check(s) did not fire: {", ".join(missed)}')
    if not clean_ok:
        print('SELF-TEST FAILED: the clean control produced findings (false positives).')
    return 1


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('path', nargs='?', help='HANDOFF.md to check (default: <repo>/HANDOFF.md)')
    ap.add_argument('--repo', help='repository root (default: found by walking up)')
    ap.add_argument('--self-test', action='store_true', help='verify the checker itself')
    ap.add_argument('-v', '--verbose', action='store_true', help='show how facts were derived')
    args = ap.parse_args(argv)

    try:                       # em dashes are everywhere in this document
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')  # type: ignore[attr-defined]
    except Exception:
        pass

    if args.self_test:
        return self_test()

    repo = Path(args.repo).resolve() if args.repo else None
    if repo is None:
        seed = Path(args.path).resolve().parent if args.path else Path.cwd()
        repo = find_repo(seed) or find_repo(Path.cwd())
    if repo is None or not (repo / 'progress.py').exists():
        print('error: could not locate the repository root; pass --repo', file=sys.stderr)
        return 2

    md = Path(args.path).resolve() if args.path else repo / 'HANDOFF.md'
    if not md.exists():
        print(f'error: {md} not found', file=sys.stderr)
        return 2

    facts = load_facts(repo)
    doc = parse_doc(md.read_text(encoding='utf-8', errors='replace'))
    return report(run_checks(doc, facts), facts, md, args.verbose)


if __name__ == '__main__':
    sys.exit(main())
