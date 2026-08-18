from pathlib import Path
import sys;sys.path.insert(0,'scratch/codex_round10');import harness
s=Path('scratch/codex_round10/createCourseInit/intaction.cpp').read_text();a=s.index('void daPyMng_c::createCourseInit()');b=s.index('// .sdata2 literal',a);Path('scratch/codex_round10/createCourseInit/RESULT.md').write_text('# createCourseInit Round 10 Result

Status: NEAR-MISS.

Final source (strongest measured variant): `intaction.cpp`, with `int action = getPlayerCreateAction();`; the full function source is between `void daPyMng_c::createCourseInit()` and the following `.sdata2` literal section.

Measured whole-TU result: target 352 instructions, draft 347 instructions (0x56c bytes vs target 0x580), 5 instructions short. The variant restores the target three independent action compares (`cmpwi`/`beq`) but still differs in the initial materialisation and subsequent register/frame schedule. Baseline was 345 instructions with `u8 action`, range-folded `cmplwi/ble`, and bool bit-trick materialisation.

getFileP status: emitted as an out-of-line `bl getFileP__5dCd_cFi` in the 347-instruction variant, confirming the size-coupling theory directionally. It is absent from the 345-instruction baseline.

Exact source:

```cpp
'+s[a:b]+'```

Diff: not byte-exact. First differences in the 347-instruction variant are `_savegpr_26` vs target `_savegpr_27`, then action setup/branch offsets and local stack/register scheduling. The target begins `clrlwi. r31, r3, 24`; the variant begins `cmpwi r3, 0; mr r31, r3`.

Confidence: medium. The source is compile-verified whole-TU and proves that integer action type adds 2 instructions and causes `getFileP` to be emitted, but no tested spelling reached the target 352-byte-exact body.
',encoding='ascii')
