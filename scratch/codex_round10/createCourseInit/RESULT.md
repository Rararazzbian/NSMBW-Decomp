# createCourseInit Round 10 Result

Status: NEAR-MISS.

Final source: `intaction.cpp` in this directory. It is the complete whole-TU source copy with `int action = getPlayerCreateAction();`.

Measured whole-TU result: target 352 instructions, draft 347 instructions, or 0x56c bytes versus the target 0x580. The original baseline was 345 instructions with `u8 action`, range-folded `cmplwi/ble`, and bool bit-trick materialisation.

The integer-action variant restores three independent action comparisons and adds two instructions. It is not byte-exact. The first differences are target `_savegpr_27` versus draft `_savegpr_26`, followed by action setup, branch offsets, and stack/register scheduling. Target begins `clrlwi. r31, r3, 24`; the variant begins `cmpwi r3, 0; mr r31, r3`.

getFileP status: the 347-instruction variant emits an out-of-line `bl getFileP__5dCd_cFi`. It is absent from the 345-instruction baseline. This confirms the size-coupling theory directionally, but the function remains five instructions short.

Confidence: medium. The result is compile-verified using the whole assembled TU and the harness. No tested spelling reached byte-exact output.

## Final source

See `intaction.cpp` for the complete source used for the final measurement.
