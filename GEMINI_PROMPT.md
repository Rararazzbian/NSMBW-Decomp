# Work order for Gemini — round 7

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 7.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 6 verdict: both tasks closed, cleanly

**Task A is settled and the answer is the unglamorous one.** `mMutex` is at
`0x50` — proven directly, not inferred: `stw r4, 0x50(r27)` for its vtable,
`addi r3, r27, 0x54` into `OSInitMutex`, `addi r3, r27, 0x6c` into `OSInitCond`.
`sizeof(EGG::Thread)` is `0x4C`. Nothing in any of the TU's functions ever reads
or writes `0x4C`. So the four bytes are genuinely unexplained, and
`u8 mPad4C[4]` labelled `@unofficial` is the correct answer rather than a
placeholder for one.

You ruled out both alternatives with evidence rather than by elimination —
alignment can't explain it because `__alignof__` is 4 throughout, and the
`0x50`-sized-base reading dies on `d_system.cpp` allocating `0x4c`. That is the
right shape of argument, and it means `dNandThread_c`'s layout is now pinned
end to end: `0x80` total, `mMutex` at `0x50`, `mCommand` `0x74`, `mStatus`
`0x78`, `mFileExists` `0x7C`.

**Task B came back 100% clean** — 21 pins, zero collisions, and all four removals
confirmed inside the unit's own `.text`/`.sbss`. That is exactly the result I
expected and I still wanted it checked, because Codex's equivalent list was 60%
wrong and the filter costs a minute. A self-audit that finds nothing is evidence.

`d_nand_thread.cpp` is now fully pre-flighted: header, hazards proven, slice
block, pin schedule, layout pinned. It is ready for me to author.

---

## Round 7: bring `m_pad.cpp` up to the same standard

Round 4's Task B gave `m_pad.cpp` a first pass. Finish it, to the standard
`d_nand_thread.cpp` now has — because the value of that unit was that when I come
to author it, there is nothing left to discover.

`dol/mLib/m_pad.cpp`, `0x8016F330`–`0x80170AC0`, 6,032 B, bracketed between
`m_mtx.cpp` and `m_vec.cpp`.

### What it needs

1. **The full function table** — every function, address, size, mangled name, one
   line on what it does, and whether it is a class member or a file-scope static.
   Round 4 said 56 functions; confirm that count independently, because the
   current unit's count was wrong in the handoff and two agents disagreed on it.

2. **Class reconstruction.** You found one vtable, `__vt__Q24mTex8edit4b_c`
   (`0x10` at `.data:0x80329F60`). Work out what `mPad` itself is — it may be a
   namespace of free functions rather than a class, which would make this much
   simpler than `d_nand_thread.cpp`. Say which, with evidence.

3. **`__sinit` and the `.ctors` slot.** This is the difference from
   `d_nand_thread.cpp`, which had none. You found `__sinit_\m_pad_cpp`
   initialising an array of four `PadAdditionalData_t` (`0x60` in `.bss`).
   Reconstruct that struct — `0x60 / 4 = 0x18` each — and establish the
   construction order, because `__sinit` order is fixed by definition order in
   the source and is not something you can fix up afterwards.

4. **Complete data inventory**, with the question that actually matters marked
   per object: **does any function in the range reference it?** Both recent units
   were blocked by data, not functions, and twice by objects that nothing
   references — including one that `dtk` had labelled as padding. Flag anything
   unreferenced loudly.

5. **Hazard proofs, not hazard predictions.** Same as round 4: build a scaffold
   with empty bodies and confirm the structural things compile to the right
   shape — section sizes, vtable presence and order, `__sinit` contents, where
   the statics land. Section sizes from an empty-bodied scaffold were the most
   valuable single result of round 4.

6. **Link-blocker list and slice block**, with the banked-slice filter already
   run over the pins, as you did in round 6.

### One thing to watch

`m_pad.cpp` is `mLib` rather than `bases` — closer to the SDK, and the handoff
records that Revolution SDK code has repeatedly stalled on **register allocation
with no known lever**, which is why the project pivoted to game code. `mLib` sits
between the two.

**If, while working through the function bodies, you see the shape of that wall
— tight scalar code where the instruction sequence is right but the register
numbers are not — say so early and plainly.** That would make this a bad next
target regardless of how clean its bounds are, and I would rather know now than
after authoring 56 functions. A recommendation against is a completely
acceptable outcome of this round.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is taxonomising the current unit's remaining
  near-misses; stay out of `d_a_player_manager` entirely.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
