#pragma once
#include <game/sLib/s_StateMgr.hpp>

// SHADOW OVERRIDE of include/game/sLib/s_StateStateMgr.hpp.  PROPOSED shared-
// header change -- must be validated by the lead with `progress.py --verify-bin`
// before it lands.  Nothing in include/ was edited.
//
// Finding: retail has NO abstract `sStateStateMgrIf_c` intermediate base.
//  * `d_line_mng.cpp`'s .data reserves only 0x80 for its weak interface vtable
//    block (the zero-filled hole 0x80316FA8..0x80317028); the in-tree header
//    makes MWCC emit 0xC0 there, the extra 0x40 being `__vt__18sStateStateMgrIf_c`.
//  * That 0x40 shifted every subsequent .data displacement in
//    `__sinit_\d_line_mng_cpp` by +0x40 -- 175 of its 1193 instructions, and
//    NOTHING else.  Removing the base closed all 175.
//  * With the base gone the unit's whole .data is 0xA98, exactly retail's
//    0x80316CA0..0x80317738.
//  * `sStateStateMgrIf_c` appears NOWHERE in bin/dtk/wiimj2d_symbols.txt --
//    neither its vtable nor its destructor.
//  * Control: the landed byte-exact `dol/bases/d_actor_state.cpp` shows the
//    same 0x80 five-vtable interface block zero-filled in retail, proving the
//    MW linker zero-fills dead weak duplicates in .data rather than compacting,
//    so a hole size IS the compiler's emitted byte count.
//
// The four extra virtuals are therefore declared here, in retail's VTABLE SLOT
// ORDER (changeToSubState=10, returnState=11, isSubState=12, getMainStateID=13,
// read off __vt__91sStateStateMgr_c<10dLineMng_c,...> at 0x80316ED8).
//
// KNOWN RESIDUAL: MWCC emits inline members in declaration order, so this
// ordering also swaps the .text emission order of isSubState (retail 0x800C72E0,
// 0x18) and changeToSubState (retail 0x800C7470, 0x1C).  Retail wants slot order
// and emission order to DISAGREE, which single inheritance cannot express.  See
// the round report; unresolved.
template <class T, template <class, class> class Manager, class Method1, class Method2>
class sStateStateMgr_c : public sStateMgrIf_c {
public:
    sStateStateMgr_c(T &owner, const sStateIDIf_c &initialState) :
        mainMgr(owner, initialState),
        subMgr(owner, initialState),
        currentMgr(&mainMgr) {}

    virtual void initializeState() { currentMgr->initializeState(); }
    virtual void executeState() { currentMgr->executeState(); }
    virtual void finalizeState() {
        if (isSubState()) {
            returnState();
        } else {
            currentMgr->finalizeState();
        }
    }

    virtual void changeToSubState(const sStateIDIf_c &newState) {
        currentMgr = &subMgr;
        currentMgr->changeState(newState);
    }

    virtual void returnState() {
        if (isSubState()) {
            currentMgr->finalizeState();
            currentMgr = &mainMgr;
        }
    }

    virtual const sStateIDIf_c *getOldStateID() const { return currentMgr->getOldStateID(); }

    virtual void refreshState() { currentMgr->refreshState(); }

    virtual bool isSubState() const { return currentMgr == &subMgr; }

    virtual void changeState(const sStateIDIf_c &newState) { currentMgr->changeState(newState); }
    virtual sStateIf_c *getState() const { return currentMgr->getState(); }
    virtual const sStateIDIf_c *getNewStateID() const { return currentMgr->getNewStateID(); }
    virtual const sStateIDIf_c *getStateID() const { return currentMgr->getStateID(); }
    virtual const sStateIDIf_c *getMainStateID() const { return mainMgr.getStateID(); }

    Manager<T, Method1> mainMgr;
    Manager<T, Method2> subMgr;
    sStateMgrIf_c *currentMgr;
};
