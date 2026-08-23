import sys, re
sys.path.append('.')
from scratch.gemini_round24 import tool

CPP_PATH = 'scratch/gemini_round24/d_enemy_toride_kokoopa.cpp'
with open(CPP_PATH, 'r', encoding='utf-8') as f:
    orig_code = f.read()

target_all = {}
for tf in tool.TARGET_FILES:
    target_all.update(tool.parse_disasm(tf))
t_fn = target_all.get('hitCallback_PenguinSlide__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c')

variations = [
    # 0: direct ternary
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, (mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
    # 1: if-else on member
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c::DamageType_e dmg = (daPlBase_c::DamageType_e)3;
    if (mUnk794 & 2) dmg = (daPlBase_c::DamageType_e)2;
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, dmg);
    return true;
}""",
    # 2: if-else reversed
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c::DamageType_e dmg = (daPlBase_c::DamageType_e)2;
    if (!(mUnk794 & 2)) dmg = (daPlBase_c::DamageType_e)3;
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, dmg);
    return true;
}""",
    # 3: int dmg
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    int dmg = (mUnk794 & 2) ? 2 : 3;
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, (daPlBase_c::DamageType_e)dmg);
    return true;
}""",
    # 4: function call ternary
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, (daPlBase_c::DamageType_e)((mUnk794 & 2) ? 2 : 3));
    return true;
}""",
    # 5: otherCc->getOwner() first
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c *pl = (daPlBase_c*)otherCc->getOwner();
    pl->setDamage(this, (mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
    # 6: otherCc->getOwner() first with if
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c *pl = (daPlBase_c*)otherCc->getOwner();
    daPlBase_c::DamageType_e dmg = (daPlBase_c::DamageType_e)3;
    if (mUnk794 & 2) dmg = (daPlBase_c::DamageType_e)2;
    pl->setDamage(this, dmg);
    return true;
}""",
    # 7: (dActor_c*)this
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c *pl = (daPlBase_c*)otherCc->getOwner();
    pl->setDamage((dActor_c*)this, (mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
    # 8: comma operator / evaluation order
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, (this->mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
    # 9: static cast
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    static_cast<daPlBase_c*>(otherCc->getOwner())->setDamage(this, (mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
    # 10: volatile or reference
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    daPlBase_c::DamageType_e dmg = (daPlBase_c::DamageType_e)3;
    if (this->mUnk794 & 2) { dmg = (daPlBase_c::DamageType_e)2; }
    ((daPlBase_c*)otherCc->getOwner())->setDamage(this, dmg);
    return true;
}""",
    # 11: temporary this pointer
    """bool dEnTorideKokoopa_c::hitCallback_PenguinSlide(dCc_c *myCc, dCc_c *otherCc) {
    dEnTorideKokoopa_c *self = this;
    ((daPlBase_c*)otherCc->getOwner())->setDamage(self, (self->mUnk794 & 2) ? (daPlBase_c::DamageType_e)2 : (daPlBase_c::DamageType_e)3);
    return true;
}""",
]

fn_pat = re.compile(r'bool dEnTorideKokoopa_c::hitCallback_PenguinSlide\([^)]*\)\s*\{[^}]*\}', re.DOTALL)

for i, var in enumerate(variations):
    new_code = fn_pat.sub(var, orig_code)
    with open(CPP_PATH, 'w', encoding='utf-8') as f:
        f.write(new_code)
    if not tool.compile_and_disasm():
        print(f"Var {i}: Compile error")
        continue
    draft_all = tool.parse_disasm(tool.DRAFT_DIS)
    d_fn = draft_all.get('hitCallback_PenguinSlide__18dEnTorideKokoopa_cFP5dCc_cP5dCc_c')
    if d_fn:
        diffs = sum(1 for (tb, tt), (db, dt) in zip(t_fn, d_fn) if tb != db and tt != dt) + abs(len(t_fn) - len(d_fn))
        raw_diffs = sum(1 for (tb, _), (db, _) in zip(t_fn, d_fn) if tb != db) + abs(len(t_fn) - len(d_fn))
        print(f"Var {i}: {raw_diffs} raw diffs, {diffs} canonical diffs")
        if raw_diffs == 0:
            print(f"*** Var {i} MATCHES 100%! ***")
            break
        else:
            # print instruction 4
            if len(d_fn) > 4:
                print(f"   Insn 4: D=[{d_fn[4][0]}] {d_fn[4][1]} vs T=[{t_fn[4][0]}] {t_fn[4][1]}")

# restore
with open(CPP_PATH, 'w', encoding='utf-8') as f:
    f.write(orig_code)
tool.compile_and_disasm()
