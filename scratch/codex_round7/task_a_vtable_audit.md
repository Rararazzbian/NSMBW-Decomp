# EGG::Effect vtable slot audit

## Conclusion

The vtable and header match one-to-one. The vtable has 37 function-pointer slots, and include/lib/egg/util/eggEffect.hpp declares 37 virtual functions including the destructor. The prior claim that the header declares 35 virtuals is incorrect: the header has 37 lines beginning with virtual (lines 13-49).

Evidence: bin/dtk/wiimj2d_symbols.txt:24419 reports __vt__Q23EGG6Effect at .data:0x80350AF8 with size 0x9C. The object was disassembled from bin/dtkspl/obj/auto_07_80350240_data.o. The complete output is in scratch/codex_round7/task_a_vtable_audit_disasm.txt.

## Size calculation

(0x9C - 8) / 4 = 0x94 / 4 = 37.

The first two 4-byte words are zero at 0x80350AF8 and 0x80350AFC. Slot 1 starts at 0x80350B00 and slot 37 ends at 0x80350B93, exactly filling the 0x9C-byte object.

## Full vtable block

    .obj __vt__Q23EGG6Effect, global
        .4byte 0x00000000
        .4byte 0x00000000
        .4byte __dt__Q23EGG6EffectFv
        .4byte create__Q23EGG6EffectFv
        .4byte fade__Q23EGG6EffectFv
        .4byte followFade__Q23EGG6EffectFv
        .4byte kill__Q23EGG6EffectFv
        .4byte setDisableCalc__Q23EGG6EffectFb
        .4byte setDisableDraw__Q23EGG6EffectFb
        .4byte setDisableCalcDraw__Q23EGG6EffectFb
        .4byte setLife__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
        .4byte setEmitRatio__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
        .4byte setEmitInterval__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
        .4byte setEmitEmitDiv__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
        .4byte setInitVelocityRandom__Q23EGG6EffectFScQ33EGG6Effect10ERecursive
        .4byte setPowerYAxis__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
        .4byte setPowerRadiationDir__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
        .4byte setPowerSpecDir__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
        .4byte setPowerSpecDirAdd__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
        .4byte setSpecDir__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
        .4byte setSpecDirAdd__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
        .4byte setVelocity__Q23EGG6EffectFRCQ34nw4r4math4VEC3
        .4byte setColor__Q23EGG6EffectFUcUcUcUcQ33EGG6Effect10ERecursive
        .4byte setRegisterColor__Q23EGG6EffectFRC8_GXColorRC8_GXColorUcQ33EGG6Effect10ERecursive
        .4byte setRegisterAlpha__Q23EGG6EffectFUcUcUcQ33EGG6Effect10ERecursive
        .4byte setDefaultParticleSize__Q23EGG6EffectFRQ34nw4r4math4VEC2Q33EGG6Effect10ERecursive
        .4byte setParticleScale__Q23EGG6EffectFRQ34nw4r4math4VEC2Q33EGG6Effect10ERecursive
        .4byte setDefaultParticleRotate__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
        .4byte setParticleRotate__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
        .4byte setEmitterSize__Q23EGG6EffectFRCQ34nw4r4math4VEC3bQ33EGG6Effect10ERecursive
        .4byte setLocalScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
        .4byte setDynamicsScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3PCQ34nw4r4math4VEC2
        .4byte setScale__Q23EGG6EffectFf
        .4byte setScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3
        .4byte setPos__Q23EGG6EffectFRCQ34nw4r4math4VEC3
        .4byte setMtx__Q23EGG6EffectFRCQ34nw4r4math5MTX34
        .4byte setPtclAnim__Q23EGG6EffectFib
        .4byte update__Q23EGG6EffectFv
        .4byte reset__Q23EGG6EffectFv
    .endobj __vt__Q23EGG6Effect

## Declaration-order lists

Vtable slots, in order:

1 __dt__Q23EGG6EffectFv
2 create__Q23EGG6EffectFv
3 fade__Q23EGG6EffectFv
4 followFade__Q23EGG6EffectFv
5 kill__Q23EGG6EffectFv
6 setDisableCalc__Q23EGG6EffectFb
7 setDisableDraw__Q23EGG6EffectFb
8 setDisableCalcDraw__Q23EGG6EffectFb
9 setLife__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
10 setEmitRatio__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
11 setEmitInterval__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
12 setEmitEmitDiv__Q23EGG6EffectFUsQ33EGG6Effect10ERecursive
13 setInitVelocityRandom__Q23EGG6EffectFScQ33EGG6Effect10ERecursive
14 setPowerYAxis__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
15 setPowerRadiationDir__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
16 setPowerSpecDir__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
17 setPowerSpecDirAdd__Q23EGG6EffectFfQ33EGG6Effect10ERecursive
18 setSpecDir__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
19 setSpecDirAdd__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
20 setVelocity__Q23EGG6EffectFRCQ34nw4r4math4VEC3
21 setColor__Q23EGG6EffectFUcUcUcUcQ33EGG6Effect10ERecursive
22 setRegisterColor__Q23EGG6EffectFRC8_GXColorRC8_GXColorUcQ33EGG6Effect10ERecursive
23 setRegisterAlpha__Q23EGG6EffectFUcUcUcQ33EGG6Effect10ERecursive
24 setDefaultParticleSize__Q23EGG6EffectFRQ34nw4r4math4VEC2Q33EGG6Effect10ERecursive
25 setParticleScale__Q23EGG6EffectFRQ34nw4r4math4VEC2Q33EGG6Effect10ERecursive
26 setDefaultParticleRotate__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
27 setParticleRotate__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
28 setEmitterSize__Q23EGG6EffectFRCQ34nw4r4math4VEC3bQ33EGG6Effect10ERecursive
29 setLocalScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3Q33EGG6Effect10ERecursive
30 setDynamicsScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3PCQ34nw4r4math4VEC2
31 setScale__Q23EGG6EffectFf
32 setScale__Q23EGG6EffectFRCQ34nw4r4math4VEC3
33 setPos__Q23EGG6EffectFRCQ34nw4r4math4VEC3
34 setMtx__Q23EGG6EffectFRCQ34nw4r4math5MTX34
35 setPtclAnim__Q23EGG6EffectFib
36 update__Q23EGG6EffectFv
37 reset__Q23EGG6EffectFv

Header declarations, in order (include/lib/egg/util/eggEffect.hpp lines 13-49):

1 virtual ~Effect();
2 virtual void create();
3 virtual void fade();
4 virtual void followFade();
5 virtual void kill();
6 virtual void setDisableCalc(bool);
7 virtual void setDisableDraw(bool);
8 virtual void setDisableCalcDraw(bool);
9 virtual void setLife(unsigned short, EGG::Effect::ERecursive);
10 virtual void setEmitRatio(float, EGG::Effect::ERecursive);
11 virtual void setEmitInterval(unsigned short, EGG::Effect::ERecursive);
12 virtual void setEmitEmitDiv(unsigned short, EGG::Effect::ERecursive);
13 virtual void setInitVelocityRandom(s8, EGG::Effect::ERecursive);
14 virtual void setPowerYAxis(float, EGG::Effect::ERecursive);
15 virtual void setPowerRadiationDir(float, EGG::Effect::ERecursive);
16 virtual void setPowerSpecDir(float, EGG::Effect::ERecursive);
17 virtual void setPowerSpecDirAdd(float, EGG::Effect::ERecursive);
18 virtual void setSpecDir(const nw4r::math::VEC3&, EGG::Effect::ERecursive);
19 virtual void setSpecDirAdd(const nw4r::math::VEC3&, EGG::Effect::ERecursive);
20 virtual void setVelocity(const nw4r::math::VEC3&);
21 virtual void setColor(u8, u8, u8, u8, EGG::Effect::ERecursive);
22 virtual void setRegisterColor(const _GXColor &, const _GXColor &, u8, EGG::Effect::ERecursive);
23 virtual void setRegisterAlpha(u8, u8, u8, EGG::Effect::ERecursive);
24 virtual void setDefaultParticleSize(nw4r::math::VEC2&, EGG::Effect::ERecursive);
25 virtual void setParticleScale(nw4r::math::VEC2&, EGG::Effect::ERecursive);
26 virtual void setDefaultParticleRotate(const nw4r::math::VEC3&, EGG::Effect::ERecursive);
27 virtual void setParticleRotate(const nw4r::math::VEC3&, EGG::Effect::ERecursive);
28 virtual void setEmitterSize(const nw4r::math::VEC3&, bool, EGG::Effect::ERecursive);
29 virtual void setLocalScale(const nw4r::math::VEC3&, EGG::Effect::ERecursive);
30 virtual void setDynamicsScale(const nw4r::math::VEC3&, const nw4r::math::VEC2*);
31 virtual void setScale(float);
32 virtual void setScale(const nw4r::math::VEC3&);
33 virtual void setPos(const nw4r::math::VEC3&);
34 virtual void setMtx(const nw4r::math::MTX34&);
35 virtual void setPtclAnim(int, bool);
36 virtual void update();
37 virtual void reset();

The two lists match one-to-one at every position. There are no missing declarations and no extra vtable entries. No insertion is required, so no later override moves. If a real two-slot mismatch had existed, the exact shift point would have been the first unmatched slot, with every following override displaced by two slots, but no such position exists in the evidence.

## Pure-virtual audit

There are exactly two .4byte 0x00000000 entries, and both are the leading words excluded by the task. No zero pointer occurs in the 37 function slots. Therefore there is no pure-virtual stub.

