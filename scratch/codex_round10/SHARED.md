# Round 10 shared context

## What this is
You are fixing three near-miss functions in daPyMng_c (d_a_player_manager.cpp).
You work from scratch/codex_round10/assembled.cpp — you may modify YOUR COPY,
never wip/. Compile the WHOLE assembled.cpp, not a one-function draft.

## The compiler
Use harness.py's compile_draft/extract/diff_fn. NEVER hand-build a command line.
MWCC: -proc gekko -fp hard -O4 -inline noauto -Cpp_exceptions off -enum int -RTTI off -ipa file

## Hard rules
- Never run ninja, configure.py, progress.py, land.py
- Never edit a shared header, slices/wiimj2d.json, or syms.txt — propose
- Do not touch wip/ (read freely, shadow-copy, write nothing)
- Report contradictions rather than reconciling them
- Plain ASCII or clean UTF-8, LF, no BOM

## The functions
Target text is target_text.txt. ASSEMBLY.md is the authority for current status.
SHARED-BRIEF.md has levers and known mechanisms.
