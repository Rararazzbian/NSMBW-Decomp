# Round 5, Task A: dPyEffect_c reconstruction

## Result

The proposed header is scratch/d_py_effect_real.hpp. It reconstructs dPyEffect_c as one virtual destructor, an embedded dEf::followEffect_c, the known position and scale vectors, the known layer byte, the known effect id, the known active flag, and three bytes of alignment padding.

The total size is preserved at 0x13C bytes. The embedded effect starts at offset 0x004 and occupies 0x114 bytes, ending at offset 0x118. The trailing members therefore land at the measured offsets 0x118, 0x124, 0x130, 0x134, and 0x138.

## Header verification

include/game/bases/d_effect.hpp declares dEf::followEffect_c as a subclass of mEf::effect_c with no data members. It has a constructor and virtual destructor only.

include/game/mLib/m_effect.hpp declares mEf::effect_c as a subclass of EGG::Effect with virtual functions but no data members.

include/lib/egg/util/eggEffect.hpp declares EGG::Effect with one vptr and u8 mPad[0x110]. Under the MWCC compiler used below, sizeof(EGG::Effect), sizeof(mEf::effect_c), and sizeof(dEf::followEffect_c) all passed static assertions for 0x114. No adjustment to the 0x114 embedded size was needed.

The disassembly in scratch/test_disasm.txt at __ct__11dPyEffect_cFv, address 0x800D2AE0, stores the dPyEffect_c vtable at object offset 0x000, calls __ct__Q23EGG6EffectFv with this + 0x004, and then stores __vt__Q23dEf14followEffect_c at the embedded object vptr. This directly supports the embedded dEf::followEffect_c member at offset 0x004.

The same disassembly writes the active flag at offset 0x138. The known field accesses place position at 0x118, 0x11C, and 0x120; scale at 0x124, 0x128, and 0x12C; layer at 0x130; and effect id at 0x134. These offsets are also consistent with the 0x114 embedded object ending at 0x118.

## Compiler test

The test source is scratch/test_d_py_effect_real.cpp. It was compiled directly with compilers\Wii\1.1\mwcceppc.exe using the build flags -proc gekko -fp hard -O4 -inline noauto -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS and the repository include directories, plus -i scratch for the proposed header.

The final compiler invocation returned exit code 0 and produced scratch/test_d_py_effect_real.o. Static assertions passed for the three effect hierarchy sizes, sizeof(dPyEffect_c) == 0x13C, and the standalone runtime expression checking both final sizes.

An earlier attempt used offsetof assertions. MWCC accepted the translation unit but emitted warning 10124, illegal constant expression, for those member offset assertions. Those assertions were removed from the final header because they are compiler limitations, not layout failures. The measured offsets above come from the disassembly and declaration arithmetic.

## Offset impact

This is non-perturbing. The live header currently represents offsets 0x004 through 0x13B with one pad array. Placing the reconstructed members inside that existing covered region does not move any following member or change sizeof(dPyEffect_c). No file under include/ was edited.

## Complete proposed header

#pragma once

#include <game/bases/d_effect.hpp>
#include <game/mLib/m_vec.hpp>

class dPyEffect_c {
public:
    virtual ~dPyEffect_c() {}

    dEf::followEffect_c mEffect;
    mVec3_c mPosition;
    mVec3_c mScale;
    u8 mLayer;
    u8 mPad131[3];
    int mEffectId;
    int mActive;
};

static_assert(sizeof(EGG::Effect) == 0x114, "EGG::Effect size wrong");
static_assert(sizeof(mEf::effect_c) == 0x114, "mEf::effect_c size wrong");
static_assert(sizeof(dEf::followEffect_c) == 0x114, "dEf::followEffect_c size wrong");
static_assert(sizeof(dPyEffect_c) == 0x13C, "dPyEffect_c size wrong");

All report text is ASCII-only.

