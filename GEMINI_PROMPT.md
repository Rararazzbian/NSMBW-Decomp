# Work order — round 35: a new unit

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Kokoopa is PARKED, not closed, and not your failure

Round 34 was 18 more variants and no movement. Across rounds 30–34 you and I
together have run roughly seventy variants on `setQuakeDead` and the twins and
moved the object by one diff. That is not a reflection on the work — the
analysis has been good and the last four rounds each produced a real, reusable
fact. It is a reflection on the problem: all three remaining functions are MWCC
register-allocation coin-flips where the source is already correct.

The state, so nothing is lost:

    d_enemy_toride_kokoopa: 248/251 matched, 30,816 / 31,876 bytes
      setQuakeDead      78 of 85 instructions identical. Every difference
                        follows from MWCC putting the zero for two sth stores
                        in callee-saved r29 instead of volatile r0 and reusing
                        it as the conditional's merge register.
      Jump / BigJump    5 diffs each, twins, one fix closes both. Retail
                        allocates speed.x before the two FP registers the
                        int-to-float conversion consumes; the draft allocates
                        it after.

Both blocker descriptions and every ruled-out shape are in `AGENT_CONTEXT.md`
and in your rounds 30–34 reports. **Do not touch `scratch/gemini_round24/` this
round.** We come back to it with fresh eyes.

One fact worth carrying away, which I found while looking for the idiom: across
**169 landed byte-exact source files there is not one `? nullptr :` or
`? NULL :` anywhere.** The landed style is an unconditional call and a cast:

    dActor_c *actor = (dActor_c *) fManager_c::searchBaseByID(mCarryActorID);

So the ternary we have been refining in `setQuakeDead` is probably not what the
original author wrote either. That is a lead for next time, not for now.

---

## Round 35 — `dPSwManager_c`, a fresh unit

    0x800D86C0    640 B    12 functions    dPSwManager_c

      20 B  __ct__13dPSwManager_cFv
      72 B  __dt__13dPSwManager_cFv
      40 B  initialize__13dPSwManager_cFv
       4 B  execute__13dPSwManager_cFv
     256 B  ProcMain__13dPSwManager_cFv
      40 B  finalize__13dPSwManager_cFv
      ... 6 more, all small

Twelve functions, one of them substantial, the rest small. Everything is named,
so there is no symbol-map problem. **A unit this size can reach 100% in a round
or two, and unlike kokoopa it can then actually land** — which is what moves the
project number.

Work in a new directory: **`scratch/gemini_pswmgr/`**.

### 1. Set up and measure before writing anything

Extract the target listing for the unit, build the compile/disassemble harness,
and get a baseline. You did this well for kokoopa; reuse that machinery. Use
`tools/auto_decomp/fndiff.py --all` as the scoreboard from the start rather than
inventing a scorer.

**Acceptance:** the target listing extracted, the harness working, and a table of
all twelve functions with target words / frame / GPR saves / FPR saves.

### 2. Decompile, smallest first

Constructor, destructor, `execute`, the accessors — take the trivial ones first
and bank them. `ProcMain` last.

Two rules that have cost this project whole rounds, so apply them from the
start:

- **Function definition order is part of the object.** Define them in the order
  the addresses run, not in logical groups.
- **Compile or it did not happen.** Every function you report gets a real object
  and a real `fndiff.py` line. Prose is not a measurement.

**Acceptance:** every one of the twelve attempted, with a `fndiff.py` line each.
Say how many are at `DIFFS 0`.

### 3. Report what the unit needs from me

If you need a header promoted, a symbol added, or a type you cannot resolve,
say so precisely — the retail address and size, and what you need declared. That
is the manifest that lets me land it.

**Acceptance:** a landing readiness statement — is the unit landable, and if
not, the exact remaining list.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Constraints

Work only in `scratch/gemini_pswmgr/`. Do not touch `scratch/gemini_round24/`,
`scratch/round33/`, `scratch/claude_kokoopa/`, `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`**, and do not attempt the landing.

---

## Reporting

- **Lead with the count at `DIFFS 0` out of 12.**
- The twelve-function target table.
- Per-function results with `fndiff.py` lines.
- **GAINED and LOST by name** — for a new unit, GAINED is everything matched.
- `poolcheck.py` output.
- The landing readiness statement.
- `NOT REACHED`, if anything.
