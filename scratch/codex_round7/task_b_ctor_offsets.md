# EGG::Effect constructor offset audit

All four target functions were disassembled from the containing objects with bin/dtk-windows-x86_64.exe elf disasm. Stack-frame saves/restores are omitted below; the lists contain stores to the constructed object or its subobject. Offsets are relative to the original object pointer passed to the constructor.

## EGG::Effect::Effect() at 0x802D7D90, size 0x74

Containing object: bin/dtkspl/obj/auto_03_802D72FC_text.o.

Direct object stores:

- stw r4, 0x0(r3): destination 0x00..0x03, value __vt__Q23EGG6Effect.
- stw r31, 0x28(r3): destination 0x28..0x2B, value 0.
- stb r31, 0x04(r30): destination 0x04, value 0.
- stw r31, 0x24(r30): destination 0x24..0x27, value 0.

The constructor also calls nw4r::ef::HandleBase::HandleBase() at object offset 0x74 and EGG::ExEffectParam::ExEffectParam() at offset 0x7C; their internal stores are not direct store instructions in this function.

For the requested comparison ranges, the base writes 0x04, 0x24, and 0x28. It does not directly write 0x05..0x07, 0x08..0x23, or 0x2C..0x40.

## dEf::followEffect_c::followEffect_c() at 0x8009AEC0, size 0x3C

Containing object: bin/dtkspl/obj/auto_03_8009A8FC_text.o.

Direct object stores:

- stw r4, 0x0(r31): destination 0x00..0x03, value __vt__Q23dEf14followEffect_c.

This constructor calls the base with the unchanged object pointer. It adds no stores at 0x04, 0x05..0x07, 0x08..0x23, 0x24, 0x28, or 0x2C..0x40.

## mEf::levelEffect_c::levelEffect_c() at 0x800A8AB0, size 0x58

Containing object: bin/dtkspl/obj/auto_03_800A8710_text.o.

Direct object stores:

- stw r3, 0x0(r31): destination 0x00..0x03, value __vt__Q23mEf13levelEffect_c.
- stw r0, 0x114(r31): destination 0x114..0x117, value 0.
- stw r0, 0x118(r31): destination 0x118..0x11B, value 0.
- stb r0, 0x11C(r31): destination 0x11C, value 0.
- stb r0, 0x11D(r31): destination 0x11D, value 0.
- stw r0, 0x120(r31): destination 0x120..0x123, value 0.
- stw r0, 0x124(r31): destination 0x124..0x127, value 0.

This constructor calls the base with the unchanged object pointer. It adds no stores in any requested comparison range.

## dPyEffect_c::dPyEffect_c() at 0x800D2AE0, size 0x60

Containing object: bin/dtkspl/obj/auto_03_800D09F8_text.o.

This constructor saves the original object pointer in r30, sets r3 = original + 4, and calls EGG::Effect::Effect(). Therefore, stores emitted by the base are translated by +4 relative to the original dPyEffect_c object. The constructor also sets the outer vtable at offset 0 and the embedded Effect/follow-effect vtable at offset 4.

Stores expressed relative to the original object:

- stw r4, 0x0(r3): destination 0x00..0x03, value __vt__11dPyEffect_c.
- Base stw r4, 0x0(r3): destination 0x04..0x07, value __vt__Q23EGG6Effect initially; the embedded follow-effect vtable then replaces it.
- Base stw r31, 0x28(r3): destination 0x2C..0x2F, value 0.
- Base stb r31, 0x04(r30): destination 0x08, value 0.
- Base stw r31, 0x24(r30): destination 0x28..0x2B, value 0.
- stw r3, 0x0(r31): destination 0x04..0x07, value __vt__Q23dEf14followEffect_c.
- stw r0, 0x138(r30): destination 0x138..0x13B, value 0.

The stb at original object offset 0x08 is the only store by any examined constructor in the requested 0x08..0x23 range. Its value is zero. It is not evidence of a separately initialized semantic field in the ordinary EGG::Effect layout: it is the base constructor byte at offset 0x04, applied to the embedded Effect subobject beginning at original offset 0x04. Its semantic role is therefore the existing Effect byte at subobject offset 0x04, likely a boolean/status byte based on the byte store, but no stronger field name is justified by these constructors alone.

For the other requested comparison ranges, this constructor writes 0x04..0x07 as the embedded vtable, 0x28..0x2B, and 0x2C..0x2F. It does not write original offsets 0x05..0x07 as independent scalar fields; those bytes are part of the vtable pointer.

## Conclusion

The derived-constructor hypothesis is negative for dEf::followEffect_c and mEf::levelEffect_c: neither writes 0x08..0x23. It is positive only through the unusual dPyEffect_c layout. dPyEffect_c invokes the EGG::Effect base constructor at original offset 0x04, causing the base zero byte store at subobject offset 0x04 to land at original object offset 0x08. No examined constructor directly initializes any other offset in 0x08..0x23.

Confidence: high for instruction-level offsets and values. The semantic label of the byte at original dPyEffect_c offset 0x08 remains provisional. No source/header files were edited, and the report is not offset-perturbing.
