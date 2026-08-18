# Round 8 Task A

## Shadow header diff

The shadow header is identical except these removed lines:

    -        ~Vector2f() {}
    -
    -        ~Vector3f() {}
    -

No constructors, includes, or other content changed.

## Compile and disassembly

harness.compile_draft succeeded (COMPILE_OK=True). Warnings at draft lines 546 and 983: (10184) return value expected.

harness.disasm succeeded (DISASM_OK=True) with no output.

- __dt__Q23EGG8Vector2fFv: absent
- __dt__Q23EGG8Vector3fFv: absent
- __dt__7mVec2_cFv: emitted
- __dt__7mVec3_cFv: absent in this draft, contrary to the expected result

Removing the two empty inline EGG destructors eliminates both suspected spurious EGG destructor functions. The mVec2 destructor remains emitted.

## Near-miss comparisons

All comparisons against wip/player_manager/target_text.txt failed:

- incCoin__9daPyMng_cFi: FAIL, target 130 instructions, draft 126. First differences: target lis r31, m_playerID__9daPyMng_c@ha, draft lis r31, SYM0@ha; target branch blt, draft bge.
- addRest__9daPyMng_cFiib: FAIL, both 74 instructions. First differences are register allocation: target uses r4/r3/r5, draft uses r5/r4/r6 at diff lines 50-52.
- deleteCullingYoshi__9daPyMng_cFv: FAIL, both 86 instructions. First differences: target saves r29/r28 before initializing them, draft initializes r30 then saves registers; target has fmuls f1, f1, f0, draft has fmuls f1, f0, f1.

## Python code run

    import sys
    from pathlib import Path
    sys.path.insert(0, str(Path("tools/auto_decomp").resolve()))
    import harness
    root = Path.cwd()
    scratch = root / "scratch" / "codex_round8"
    ok, log = harness.compile_draft(str(scratch / "draft.cpp"), str(scratch / "draft.o"), extra_inc=(str(scratch / "include"),))
    print("COMPILE_OK=", ok, log)
    if ok:
        dok, dlog = harness.disasm(str(scratch / "draft.o"), str(scratch / "draft_text.txt"))
        print("DISASM_OK=", dok, dlog)
        for name in ["__dt__Q23EGG8Vector2fFv", "__dt__Q23EGG8Vector3fFv", "__dt__7mVec2_cFv", "__dt__7mVec3_cFv"]:
            print(name, "EMITTED" if harness.extract(str(scratch / "draft_text.txt"), name) is not None else "ABSENT")
        target = root / "wip" / "player_manager" / "target_text.txt"
        for name in ["incCoin__9daPyMng_cFi", "addRest__9daPyMng_cFiib", "deleteCullingYoshi__9daPyMng_cFv"]:
            print(name, harness.diff_fn(str(target), str(scratch / "draft_text.txt"), name))

