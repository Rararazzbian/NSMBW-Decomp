from pathlib import Path
base = Path('scratch/round30/d_bg_ctr/calc_v1_decl_order.cpp').read_text()
old_order = '''        f32 sin = nw4r::math::SinIdx(rot);\n        f32 cos = nw4r::math::CosIdx(rot);'''
new_order = '''        f32 cos = nw4r::math::CosIdx(rot);\n        f32 sin = nw4r::math::SinIdx(rot);'''
assert old_order in base
order = base.replace(old_order, new_order, 1)
Path('scratch/round30/d_bg_ctr/calc_order.cpp').write_text(order)
# The present draft has direct stores after revisePos. The store-before-call lens means
# materialise the three actor-position copies before the call and remove the copies after it.
old_stores = '''    revisePos();\n\n    *(f32 *)((u8 *)this + 0x94) = *(const f32 *)((const u8 *)mpActor + 0xAC);\n    *(f32 *)((u8 *)this + 0x98) = *(const f32 *)((const u8 *)mpActor + 0xB0);\n    *(f32 *)((u8 *)this + 0x9C) = *(const f32 *)((const u8 *)mpActor + 0xB4);'''
new_stores = '''    *(f32 *)((u8 *)this + 0x94) = *(const f32 *)((const u8 *)mpActor + 0xAC);\n    *(f32 *)((u8 *)this + 0x98) = *(const f32 *)((const u8 *)mpActor + 0xB0);\n    *(f32 *)((u8 *)this + 0x9C) = *(const f32 *)((const u8 *)mpActor + 0xB4);\n\n    revisePos();'''
assert old_stores in base
store = base.replace(old_stores, new_stores, 1)
Path('scratch/round30/d_bg_ctr/calc_store.cpp').write_text(store)
Path('scratch/round30/d_bg_ctr/calc_both.cpp').write_text(order.replace(old_stores, new_stores, 1))
