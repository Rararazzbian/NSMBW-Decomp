#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/framework/f_manager.hpp>

// ---------------------------------------------------------------------
// Unnamed daPlBase_c field this batch touches. Offset proven by this TU's
// own disassembly (see the per-field comment at each use site); left as a
// raw cast per house style, since d_a_player_base.hpp is a shared header
// this batch may not edit. Signed: the target compares it with `cmpwi`,
// not `cmplwi`, which only happens for a signed comparand.
// ---------------------------------------------------------------------
static inline s8 &scroll_flag_ref(dAcPy_c *p) {
    // 0x153c: read in getActScrollInfo and getScrollNum.
    return *reinterpret_cast<s8 *>(reinterpret_cast<u8 *>(p) + 0x153c);
}

// ---------------------------------------------------------------------
// getYoshi's inner dispatch: `lwz r12,0x60(this); lwz r12,0x6c(r12); mtctr
// r12; bctrl`, then `lbz`+`extsb` the returned pointer's byte 0. This is
// fBase_c's own vtable, whose pointer sits at object offset 0x60 (not 0)
// per HANDOFF.md ("fBase_c's vtable pointer is at object offset 0x60, not
// 0"); the call at raw slot 0x6c returns a small-int reference dereferenced
// right after -- the exact shape of `dActor_c::getPlrNo()`
// (`virtual s8 &getPlrNo() { return mPlayerNo; }`, d_actor.hpp:105), and
// "does the yoshi's rider match plrNo" is exactly what getYoshi needs.
//
// Calling it BY NAME (`((dActor_c*)base)->getPlrNo()`) reproduces getYoshi's
// 39 instructions exactly, confirming the identification -- but it also
// makes MWCC instantiate a local weak `getPlrNo__8dActor_cFv` (0x8 bytes) in
// THIS object, because getPlrNo is defined inline in the class body. The
// target has no such symbol: the gap between getYoshi and getYoshiNum is
// exactly 4 padding bytes (`gap_03_8005FAFC_text`, size 0x4), too small to
// hold it, and `grep -rn getPlrNo` over target_text.txt's whole 0x8005E9A0-
// 0x80061310 range is empty. So the real TU reached this vtable slot
// WITHOUT ODR-using the named method -- reproduced here as a raw, untyped
// vtable-slot fetch instead. See BATCH4.md for the full writeup and the
// two candidate sources.
// ---------------------------------------------------------------------
typedef s8 &(*GetPlrNoFn)(dActor_c *);
static inline GetPlrNoFn get_vfunc_6c(fBase_c *base) {
    return (*(GetPlrNoFn **)((u8 *)base + 0x60))[0x6c / 4];
}

daYoshi_c *daPyMng_c::getYoshi(int plrNo) {
    fBase_c *base;
    for (int i = 0; i < 4; i++) {
        base = fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]);
        if (base != nullptr) {
            if (get_vfunc_6c(base)((dActor_c *)base) == plrNo) {
                return (daYoshi_c *)base;
            }
        }
    }
    return nullptr;
}

int daPyMng_c::getYoshiNum() {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[i]) != nullptr) {
            count++;
        }
    }
    return count;
}

daYoshi_c *daPyMng_c::getYoshiDirectP(int idx) {
    return (daYoshi_c *)fManager_c::searchBaseByID((fBaseID_e)m_yoshiID[idx]);
}

dAcPy_c *daPyMng_c::getCtrlPlayer(int plrNo) {
    dAcPy_c *player = getPlayer(plrNo);
    if (player == nullptr) {
        return nullptr;
    }
    daYoshi_c *yoshi = player->getRideYoshi();
    if (yoshi != nullptr) {
        return (dAcPy_c *)yoshi;
    }
    return player;
}

dPyMdlMng_c::ModelType_e daPyMng_c::getCourseInPlayerModelType(u8 idx) {
    static const dPyMdlMng_c::ModelType_e scModelTypeDt[4] = {
        (dPyMdlMng_c::ModelType_e)0,
        (dPyMdlMng_c::ModelType_e)1,
        (dPyMdlMng_c::ModelType_e)2,
        (dPyMdlMng_c::ModelType_e)3,
    };
    PLAYER_TYPE_e type = mPlayerType[idx];
    if (mCreateItem[type] & 0x8) {
        return (dPyMdlMng_c::ModelType_e)4;
    }
    return scModelTypeDt[type];
}

void daPyMng_c::setCarryOverYoshiInfo(u8 plrNo, u8 yoshiColor, int fruitCount) {
    m_yoshiColor[plrNo] = yoshiColor;
    m_yoshiFruit[plrNo] = fruitCount;
}

int daPyMng_c::getYoshiColor(u8 plrNo) {
    return m_yoshiColor[plrNo];
}

int daPyMng_c::getYoshiFruit(u8 plrNo) {
    return m_yoshiFruit[plrNo];
}

int daPyMng_c::getActScrollInfo() {
    int mask = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scroll_flag_ref(player) != 1) {
                    u8 bit = 1 << i;
                    mask |= bit;
                }
            } else {
                u8 bit = 1 << i;
                mask |= bit;
            }
        }
    }
    return mask;
}

int daPyMng_c::getScrollNum() {
    u8 count = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i]) {
            dAcPy_c *player = getPlayer(i);
            if (player != nullptr) {
                if (scroll_flag_ref(player) != 1) {
                    count++;
                }
            } else {
                count++;
            }
        }
    }
    return count;
}
