#pragma once
#include <game/framework/f_base_id.hpp>
#include <game/framework/f_base.hpp>
#include <game/mLib/m_vec.hpp>

class dActor_c;
class fBase_c;

class dAttention_c {
public:
    virtual ~dAttention_c();
    void entry(fBaseID_e id);
    fBase_c *search(mVec3_c pos);
    fBase_c *searchPlayer(const dActor_c *player, mVec3_c pos);

    // sizeof(dAttention_c) == 0x58 (88 bytes)
    //   0x00: vtable pointer (4 bytes) ? confirmed: ctor writes __vt__12dAttention_c at 0(r3)
    //   0x04: mEntryCount (int, 4 bytes) ? reset() stw r0, 4(r3)
    //   0x08: mEntryList[10] ? fBaseID_e entries (40 bytes) ? reset loop writes 4(r3)+i*4
    //   0x30: mActiveList[10] ? fBaseID_e entries (40 bytes) ? reset loop writes 0x30(r3)+i*4

    int mEntryCount;
    fBaseID_e mEntryList[10];
    fBaseID_e mActiveList[10];

    static dAttention_c *mspInstance;
};

static_assert(sizeof(dAttention_c) == 0x58, "dAttention_c must be 0x58 bytes");
