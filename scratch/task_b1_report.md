# Round 5 Task B1 report

## Result

The requested padding array is not present in the checked-out
`include/game/bases/d_a_player_base.hpp`. The class has no pad, padding, or
unnamed byte array covering offsets `0x1000` through `0x1100`. The nearby
members are named fields, ending with `m_1134`, `m_1138`, `m_113c`, and
`mPlayerType`.

The raw access at `daPlBase_c + 0x1036` is therefore not safely authorable as
a split of a padding member in this header. This is a source/layout mismatch
with the task premise, not evidence that `0x1036` is unused. The target
disassembly contains byte loads and stores at exactly `0x1036`, and the batch
context identifies it as the per-actor Yoshi priority rank.

## Offset arithmetic

No padding-array start can be calculated because no such array exists. There
is no valid expression of the requested form `pad_start + index = 0x1036`.
The nearby markers provide only these verified arithmetic anchors:

```text
0x1036 + 1 = 0x1037
0x1134 + 4 = 0x1138
0x1138 + 4 = 0x113c
0x113c + 4 = 0x1140
```

They do not identify the bytes from `0x1036` through `0x1133` as a declared
array. Inventing a pad start or length could move every following member.
The `m_103c` name in `include/game/bases/d_a_player.hpp` is in the derived
player class and is not a padding member in `daPlBase_c`.

## Split safety

The element type of a containing array cannot be determined because the array
is absent. If the intended upstream header has a `u8` array, the exact split
would be safe only after its start and total length are verified:

```cpp
u8 pad_before[0x1036 - PAD_START];
u8 mYoshiPriority;
u8 pad_after[PAD_SIZE - (0x1036 - PAD_START) - 1];
```

The total remains unchanged:
`(0x1036 - PAD_START) + 1 + (PAD_SIZE - (0x1036 - PAD_START) - 1) = PAD_SIZE`.

If the actual array is `u16` or `u32`, it cannot be split at byte offset
`0x1036` unless that offset is on an element boundary. Replacing an interior
byte with `u8` changes the declared element layout and can change alignment.
The safest representation then is to keep the original array unchanged and
retain the raw byte reference.

## Proposed patch text

No direct header patch is proposed because the required containing member is
missing. Once the correct declaration is located, the patch shape is:

```diff
-    u8 pad[PAD_SIZE];
+    u8 pad_before[0x1036 - PAD_START];
+    u8 mYoshiPriority;
+    u8 pad_after[PAD_SIZE - (0x1036 - PAD_START) - 1];
```

Recommended assertions are:

```cpp
static_assert(sizeof(daPlBase_c) == EXPECTED_DA_PLBASE_SIZE,
              "daPlBase_c size changed");
static_assert(offsetof(daPlBase_c, mYoshiPriority) == 0x1036,
              "mYoshiPriority offset changed");
```

MWCC 1.1 was invoked with the player-base flags from `build.ninja`. Its
`offsetof` and `sizeof` results were rejected as illegal constant expressions
in the probe, so no compiler-confirmed value was obtained. No ninja,
configure, progress, or land command was run.

## Offset-perturbing status

The split is non-perturbing only when both pad lengths plus the named byte
equal the original padding size. A wrong total is offset-perturbing and
catastrophic. For this checked-out header, keep the raw cast as the fallback
until the missing padding declaration and exact start/size are located.
