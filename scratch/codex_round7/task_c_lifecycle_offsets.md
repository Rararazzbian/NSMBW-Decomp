# EGG::Effect lifecycle offset audit

Disassembly source: bin/dtkspl/obj/auto_03_802D72FC_text.o and bin/dtkspl/obj/auto_03_8016C3AC_text.o.

## Result for 0x08..0x23

None of the requested functions accesses EGG::Effect offsets 0x08..0x23. This is a negative result. The lifecycle functions do not explain the unobserved 28-byte region.

## Base EGG::Effect functions

### create, 0x802D7E70

- 0x802D7EE4: lwz r5, 0x24(r31), load from object offset 0x24; passes the effect resource/definition identifier to EffectManager::createEffect.
- 0x802D7E84, 0x802D7EF0, 0x802D7EF8: addi using object pointer plus 0x74, forming the embedded nw4r::ef::HandleBase address.
- No direct accesses to 0x04..0x23 or 0x25..0x40.

### fade, 0x802D7F40

- 0x802D7F54, 0x802D7F64, 0x802D7FA0, 0x802D7FAC: addi using r3/r31 plus 0x74, forming the embedded effect handle address.
- No direct loads/stores at 0x04..0x40.

### followFade, 0x802D7FD0

- 0x802D7FE4 and 0x802D8020: addi using r3/r31 plus 0x74, forming the embedded effect handle address.
- No direct loads/stores at 0x04..0x40.

### kill, 0x802D8040

- 0x802D8054, 0x802D8064, 0x802D80A0, 0x802D80AC, 0x802D80B8: addi using r3/r31 plus 0x74, forming the embedded effect handle address.
- No direct loads/stores at 0x04..0x40.

### update, 0x802D88B0

- 0x802D891C, 0x802D8944, 0x802D89C8: lwz r0, 0x28(r30), load the effect flags word. Bit 30 selects use of the object matrix at 0x44; bit 29 selects scale multiplication.
- 0x802D8954: lfs f4, 0x2C(r30), load X scale factor.
- 0x802D8984: lfs f6, 0x30(r30), load Y scale factor.
- 0x802D89AC: lfs f3, 0x34(r30), load Z scale factor.
- 0x802D89D4: lfs f0, 0x38(r30), load translation X.
- 0x802D89DC: lfs f0, 0x3C(r30), load translation Y.
- 0x802D89E4: lfs f0, 0x40(r30), load translation Z.
- 0x802D892C: addi r3, r30, 0x44, and 0x802D89FC: addi r4, r30, 0x7C, form adjacent matrix and ExEffectParam addresses.
- No stores to the object at 0x28..0x40; temporary matrix stores are stack-relative.

### reset, 0x802D8B30

- 0x802D8B50: stw r0, 0x28(r3), store zero to the effect flags word.
- 0x802D8B54, 0x802D8B58, 0x802D8B5C: stfs f1 to 0x2C, 0x30, 0x34, storing 1.0f to the three scale factors.
- 0x802D8B60, 0x802D8B64, 0x802D8B68: stfs f0 to 0x38, 0x3C, 0x40, storing 0.0f to the three translation values.
- 0x802D8B80: lwz r12, 0x7C(r31), and 0x802D8B84: addi r3, r31, 0x7C, access/form the embedded ExEffectParam.
- No access to 0x04..0x27; no access to 0x08..0x23.

## Requested mEf::effect_c helpers

### reset, 0x8016CA60

- 0x8016CA74 calls base EGG::Effect::reset with r3 unchanged, so inherited accesses are exactly those above.
- 0x8016CA78 loads 0x7C(r31), and 0x8016CA7C forms r31+0x7C for the ExEffectParam reset virtual.
- No access to 0x04..0x40 other than inherited base reset accesses.

### copyExEffectParam, 0x8016D110

- 0x8016D124, 0x8016D13C, 0x8016D154, 0x8016D198: addi using object pointer plus 0x74, forming/checking the embedded effect handle.
- 0x8016D1B0: addi r4, r31, 0x7C, forming the embedded ExEffectParam source address.
- No access to 0x04..0x40.

### vfac, 0x8016CD30

- 0x8016CDD4: addi r4, r25, 0x74, forming the effect handle address. r25 is the effect_c object pointer.
- Other low-offset accesses in this function are based on newly allocated effectCB_c/list objects in r3, r5, or r31, not on effect_c. No effect_c access to 0x08..0x23 or 0x24..0x40.

### vfa8, 0x8016CE80

- 0x8016CF34: addi r4, r23, 0x74, forming the effect handle address. r23 is the effect_c object pointer.
- As in vfac, other low-offset accesses belong to newly allocated callback/list objects. No effect_c access to 0x08..0x23 or 0x24..0x40.

## Boundary summary

- 0x04..0x07: no access in the six base lifecycle functions or four requested helpers.
- 0x08..0x23: no access by any requested function.
- 0x24: create loads lwz r5, 0x24(r31) and passes it as the effect creation parameter.
- 0x28: update loads flags and reset stores zero.
- 0x2C, 0x30, 0x34: update loads scale factors; reset stores 1.0f.
- 0x38, 0x3C, 0x40: update loads translation values; reset stores 0.0f.

The semantic names for 0x24 and 0x28..0x40 are high-confidence from instruction and call context. The 0x08..0x23 region remains unexplained and should stay padding unless another function provides direct evidence.
