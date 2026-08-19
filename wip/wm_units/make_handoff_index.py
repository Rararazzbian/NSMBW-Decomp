"""Build a readable index of HANDOFF.md.

Why this exists
---------------
HANDOFF.md is ~570KB across 10,000+ lines and 400+ sections. No agent reads it
whole, and `tail -250` shows only the newest few entries -- so agents re-derive
findings that are already recorded. That has now happened twice in one day:

  * an agent proposed proving that pool offsets self-resolve as sibling functions
    are authored. That is already project doctrine, recorded TWICE, and the
    worked example in the write-up is the very function the agent was looking at.
  * a camera-class layout gap was written up as a shared-header defect when
    landed code already solved it with a local cast.

Both cost a round's attention. The file is that size precisely so nobody pays
twice for the same answer, and it was failing at exactly that job because it had
no table of contents.

This emits `HANDOFF_INDEX.md`: every section heading with its line number, so an
agent can read a few KB, find the relevant entry, and `sed -n 'START,ENDp'` just
that section.

Usage
-----
    python make_handoff_index.py
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE = os.path.join(ROOT, "HANDOFF.md")
OUTPUT = os.path.join(ROOT, "HANDOFF_INDEX.md")

HEADING = re.compile(r"^(#{2,3})\s+(.*?)\s*$")

PREAMBLE = """# HANDOFF.md — index

HANDOFF.md is ~570KB / {lines} lines / {count} sections. **Do not read it whole,
and do not rely on `tail`** — the tail shows only the newest entries, and the
finding you need is usually older than that.

**Read this index, find the section, then read only that section:**

```
sed -n '<start>,<end>p' HANDOFF.md
grep -an "<term>" HANDOFF.md      # -a: parts of the file read as binary to grep
```

**Grep this index before designing any experiment.** Two rounds were spent in one
day re-deriving results already recorded here — once on a rule whose worked
example was the very function being looked at. The file is this size so that
nobody pays twice for the same answer.

Regenerate with `python wip/wm_units/make_handoff_index.py`.

---

"""


def main():
    with open(SOURCE, "r", encoding="utf-8", errors="replace") as handle:
        lines = handle.read().split("\n")

    sections = []
    for number, line in enumerate(lines, start=1):
        match = HEADING.match(line)
        if match:
            sections.append((number, len(match.group(1)), match.group(2)))

    out = [PREAMBLE.format(lines=len(lines), count=len(sections))]
    for index, (number, depth, title) in enumerate(sections):
        end = sections[index + 1][0] - 1 if index + 1 < len(sections) else len(lines)
        indent = "  " * (depth - 2)
        out.append("%s- `%5d-%-5d` %s" % (indent, number, end, title))

    with open(OUTPUT, "w", encoding="utf-8") as handle:
        handle.write("\n".join(out) + "\n")

    print("wrote %s: %d sections" % (os.path.relpath(OUTPUT, ROOT), len(sections)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
