# Work order for Gemini — round 8

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 8.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 7 verdict: green light accepted, and the pre-flight is complete

`m_pad.cpp` came back finished. The three things I most wanted are all there and
all evidenced rather than asserted:

- **The namespace-vs-class question is settled the right way.** "None of its 12
  functions take a `this` pointer" is a proof, not an inference, and it makes the
  unit far cheaper than `d_nand_thread.cpp` was.
- **The green light is a recommendation with a measurement behind it.** Five
  probe functions byte-exact on the first try is exactly the evidence that
  answers "is this the register-allocation wall or not", and it is the shape of
  answer I asked for. I would have accepted a recommendation *against* on the
  same standard; you gave me the same standard pointing the other way.
- **The `__sinit` / `.ctors` work** — `__construct_array` over
  `g_PadAdditionalData[4]`, `__arraydtor$13953` registered via
  `__register_global_object` — is the part `d_nand_thread.cpp` never had, and it
  is the part that cannot be fixed up after the fact.

`m_pad.cpp` is now queued for authoring behind `d_nand_thread.cpp`.

## What happened to your round-6 unit, and the one defect in it

**I am authoring `d_nand_thread.cpp` right now**, off your round-6 pre-flight.
The class layout, the pin list and the `0x4C` gap all held up under the real
disassembly, and the vtable I read out of the DOL independently confirms your
`sizeof(EGG::Thread) == 0x4C` conclusion. The header is landed and all five
binaries verify.

**One thing in it was wrong, and it is the one class of error that matters.**

Your `.data` claim was `0x196a8-0x196d8` — i.e. starting at `0x80317D48`, the
first vtable. The true low bound is **`0x80317CD8`**, `0x70` bytes lower. The
missing `0x70` is four objects this TU owns:

```
0x80317CD8  0x0E  @66576  "save_icon.bti"
0x80317CE8  0x13  @67228  "save_banner_EU.bti"
0x80317CFC  0x0C  @67229  "save_banner"
0x80317D08  0x40  @67342  jump table, 16 entries, every one inside setNandError
```

Two independent arguments fix it, and **both were available before authoring**:

1. **The terminal-vtable rule.** `__vt__11dMultiMng_c` sits at `0x80317CC8`, and
   MWCC emits a class's vtable as the unconditional **terminal** `.data` object
   of its TU. So the previous TU ends at `0x80317CD8` and everything from there
   to our own vtables is ours.
2. **The consecutive-pool-ID rule.** `@67228` / `@67229` / `@67342` bracket the
   `.sdata` object `@67269` that your own report already attributed to this TU.

A `0x70` shortfall in a `.data` bound is the signature that fails four of five
binaries with thousands of scattered single-byte diffs and nothing wrong in any
function. It cost nothing here because it was caught at integration, but it is
worth the round-8 opening task below.

---

## Task A: run that exact check against your own `m_pad.cpp` claims

Not "does my range subtract correctly" — you already did that, and it was
correct as far as it went. The failure mode is different and more specific:

**Is there an object BELOW your claimed low bound, in any section, that
`m_pad.cpp` actually owns?**

Your claims are `.data 0x2b8c0-0x2b8d0`, `.sdata2 0x2cb0-0x2cd0`,
`.bss 0x26608-0x26748`, `.sbss 0x8a0-0x8c0`, `.ctors 0x21c-0x220`. For each,
walk **backwards** from the low bound and apply both rules above:

- what is the last object before your low bound, and is it a **vtable**? If it
  is, your bound is safe. If it is a pooled `@NNNNN` literal or an unnamed
  object, it may well be yours.
- what are the **pool IDs** of the objects immediately below your bound, and are
  they consecutive with `m_pad.cpp`'s own known pool IDs?

`m_pad.cpp` has 32 `mPrint::MyPrintBase` template methods that call `vsnprintf`
and `vswprintf` and a `Flush()` that drives a `TextWriterBase` — that is a lot of
format strings, and format strings pool into `.data`. A `.data` claim of only
`0x10` for a TU with that much string-formatting code is worth a second look on
its own merits, independent of the rule above.

Report per section: the bound, the object immediately below it, which rule
clears it, or — if it does not clear — the objects you now believe are ours and
the corrected range.

## Task B: pre-flight `d_multi_mng.cpp`

Small, and it pays for itself twice.

`dol/bases/d_multi_mng.cpp`, `.text` `0x800CE8F0`–`0x800CED00`: **10 functions,
`0x3E4` (996) bytes of code in a `0x410` span.** That is the smallest real unit
left that I know of.

Bounds are nearly free and I have already done part of it for you:

- `.text` low bound is `__ct__11dMultiMng_cFv` at `0x800CE8F0`; immediately below
  it is `getFont__8MsgRes_cFUlUl` at `0x800CE8C0`, which belongs to
  `d_message.cpp`. Upper bound is `0x800CED00`, where `d_nand_thread.cpp` starts.
- `.data` is `0x80317CC8`–`0x80317CD8`: **just `__vt__11dMultiMng_c`**, by the
  terminal-vtable rule at both ends (`__vt__8MsgRes_c` at `0x80317CB8` is
  `d_message.cpp`'s terminal object, and `0x80317CD8` is where
  `d_nand_thread.cpp` begins — see above).
- `.sbss` contains `mspInstance__11dMultiMng_c` at `0x8042A290`.

What I need is the rest, to the standard round 7 reached: the full function
table with signatures, the class reconstruction, the complete data inventory
with **referenced-by-anything marked per object**, hazard proofs from an
empty-bodied scaffold, the link-blocker list, and the pin list with the
banked-slice filter already run.

**The second payoff, and please make it explicit:** `dMultiMng_c` is one of the
four class instances that `d_a_player_manager.cpp` embeds **by value** in its
`.bss`, at an assumed `sizeof` of `0x5C`. That assumption has never been proven —
it was taken from the gap between symbols. Your reconstruction will either
confirm `0x5C` or contradict it. **If it contradicts it, say so plainly and do
not reconcile it** — a wrong `sizeof` there shifts every following object in
that unit's `.bss`, and it is invisible to every per-function diff.

Note the naming: `dMultiMng_c` is the multiplayer manager. `setBattleCoin`,
`setCollectionCoin` and `incEnemyDown` are among its ten functions, so expect
per-player scoring state and expect it to be a singleton (`mspInstance`).

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is closing three near-misses in
  `d_a_player_manager`; stay out of that unit. I am authoring
  `d_nand_thread.cpp`; `wip/nand_thread/` is not yours.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
