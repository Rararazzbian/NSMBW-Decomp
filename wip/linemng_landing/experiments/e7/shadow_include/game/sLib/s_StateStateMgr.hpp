#pragma once
#include <game/sLib/s_StateMgr.hpp>

// E7 PROBE (hybrid): declaration order UNCHANGED from include/ (so vtable slot
// order is untouched).  The three members this TU calls directly -- executeState,
// changeState, getStateID -- keep their IN-CLASS bodies so MWCC still emits them
// immediately after their first-use function.  The ten that are only reachable
// through the vtable are declared bodyless and DEFINED OUT OF LINE below, in
// retail's .text emission order.
template <class T, template <class, class> class Manager, class Method1, class Method2>
class sStateStateMgr_c : public sStateMgrIf_c {
public:
    sStateStateMgr_c(T &owner, const sStateIDIf_c &initialState) :
        mainMgr(owner, initialState),
        subMgr(owner, initialState),
        currentMgr(&mainMgr) {}

    virtual void initializeState();
    virtual void executeState() { currentMgr->executeState(); }
    virtual void finalizeState();
    virtual void changeToSubState(const sStateIDIf_c &newState);
    virtual void returnState();
    virtual const sStateIDIf_c *getOldStateID() const;
    virtual void refreshState();
    virtual bool isSubState() const;
    virtual void changeState(const sStateIDIf_c &newState) { currentMgr->changeState(newState); }
    virtual sStateIf_c *getState() const;
    virtual const sStateIDIf_c *getNewStateID() const;
    virtual const sStateIDIf_c *getStateID() const { return currentMgr->getStateID(); }
    virtual const sStateIDIf_c *getMainStateID() const;

    Manager<T, Method1> mainMgr;
    Manager<T, Method2> subMgr;
    sStateMgrIf_c *currentMgr;
};

#define SSM_T template <class T, template <class, class> class Manager, class Method1, class Method2>
#define SSM_C sStateStateMgr_c<T, Manager, Method1, Method2>

SSM_T void SSM_C::initializeState() { currentMgr->initializeState(); }
SSM_T void SSM_C::finalizeState() {
    if (isSubState()) {
        returnState();
    } else {
        currentMgr->finalizeState();
    }
}
SSM_T bool SSM_C::isSubState() const { return currentMgr == &subMgr; }
SSM_T void SSM_C::returnState() {
    if (isSubState()) {
        currentMgr->finalizeState();
        currentMgr = &mainMgr;
    }
}
SSM_T const sStateIDIf_c *SSM_C::getOldStateID() const { return currentMgr->getOldStateID(); }
SSM_T void SSM_C::refreshState() { currentMgr->refreshState(); }
SSM_T void SSM_C::changeToSubState(const sStateIDIf_c &newState) {
    currentMgr = &subMgr;
    currentMgr->changeState(newState);
}
SSM_T sStateIf_c *SSM_C::getState() const { return currentMgr->getState(); }
SSM_T const sStateIDIf_c *SSM_C::getNewStateID() const { return currentMgr->getNewStateID(); }
SSM_T const sStateIDIf_c *SSM_C::getMainStateID() const { return mainMgr.getStateID(); }

#undef SSM_T
#undef SSM_C
