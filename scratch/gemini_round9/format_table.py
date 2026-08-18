import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SYMS_FILE = ROOT / 'bin' / 'dtk' / 'wiimj2d_symbols.txt'

lines = SYMS_FILE.read_text().splitlines()

tgt_fns = []
for line in lines:
    m = re.match(r'^(\S+)\s*=\s*\.text:(0x[0-9A-Fa-f]+);\s*//\s*(.*)$', line.strip())
    if m:
        name, addr_str, meta = m.groups()
        addr = int(addr_str, 16)
        if 0x800272F0 <= addr < 0x800281C0:
            size_m = re.search(r'size:(0x[0-9A-Fa-f]+)', meta)
            size = int(size_m.group(1), 16) if size_m else 0
            tgt_fns.append((addr, size, name))

tgt_fns.sort()

# Signatures map
signatures = {
    'init__14daEnCoinMain_cFv': 'void daEnCoinMain_c::init()',
    'set_coin_objbg_center__14daEnCoinMain_cFi': 'void daEnCoinMain_c::set_coin_objbg_center(int plrNo)',
    'set_coin_objbg__14daEnCoinMain_cFi': 'void daEnCoinMain_c::set_coin_objbg(int plrNo)',
    'NormalCullSizeSet__14daEnCoinMain_cFv': 'void daEnCoinMain_c::NormalCullSizeSet()',
    'coll_foot_set__14daEnCoinMain_cFi': 'void daEnCoinMain_c::coll_foot_set(int plrNo)',
    'set_bgcheck__14daEnCoinMain_cFi': 'void daEnCoinMain_c::set_bgcheck(int param)',
    'bgin_bgcheck_set__14daEnCoinMain_cFi': 'void daEnCoinMain_c::bgin_bgcheck_set(int param)',
    'sand_effect_set__14daEnCoinMain_cFv': 'void daEnCoinMain_c::sand_effect_set()',
    'beginFunsui__14daEnCoinMain_cFv': 'virtual void daEnCoinMain_c::beginFunsui()',
    'endFunsui__14daEnCoinMain_cFv': 'virtual void daEnCoinMain_c::endFunsui()',
    'bound_non_collision_check__14daEnCoinMain_cFv': 'void daEnCoinMain_c::bound_non_collision_check()',
    'setWaterSpeed__14daEnCoinMain_cFv': 'virtual void daEnCoinMain_c::setWaterSpeed()',
    'model_set__14daEnCoinMain_cFi': 'void daEnCoinMain_c::model_set(int modelType)',
    'fn_800279F0': 'void daEnCoinMain_c::fn_800279F0()',
    'angle_add__14daEnCoinMain_cFv': 'void daEnCoinMain_c::angle_add()',
    'FootBgInCheck__14daEnCoinMain_cFv': 'void daEnCoinMain_c::FootBgInCheck()',
    'drop_bgcheck__14daEnCoinMain_cFUcUcf': 'void daEnCoinMain_c::drop_bgcheck(u8 unk1, u8 unk2, f32 unk3)',
    'bg_insert_death_set__14daEnCoinMain_cFv': 'void daEnCoinMain_c::bg_insert_death_set()',
    'flash_move__14daEnCoinMain_cFv': 'void daEnCoinMain_c::flash_move()',
    'base_speed_set__14daEnCoinMain_cFv': 'void daEnCoinMain_c::base_speed_set()',
    'draw_coin__14daEnCoinMain_cFif': 'void daEnCoinMain_c::draw_coin(int drawMode, f32 scale)',
    '__dt__14daEnCoinMain_cFv': 'virtual daEnCoinMain_c::~daEnCoinMain_c()',
    '__sinit_\d_a_en_coin_main_cpp': 'static void __sinit_d_a_en_coin_main_cpp()'
}

for i, (addr, size, name) in enumerate(tgt_fns):
    sig = signatures.get(name, name)
    print(f"| {i+1} | `0x{addr:08X}` | `0x{size:03X}` | `{name}` | `{sig}` |")
