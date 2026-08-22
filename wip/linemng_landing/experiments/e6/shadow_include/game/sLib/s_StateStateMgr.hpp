#pragma once
#include <game/sLib/s_StateMgr.hpp>

// E6 PROBE: declarations in RETAIL SLOT ORDER (unchanged from include/), bodies
// moved out of the class so that DEFINITION order can drive .text emission
// order independently of slot order.
template <class T, template <class, class> class Manager, class Method1, class Method2>
class sStateStateMgr_c : public sStateMgrIf_c {
public:
    sStateStateMgr_c(T &owner, const sStateIDIf_c &initialState) :
        mainMgr(owner, initialState),
        subMgr(owner, initialState),
        currentMgr(&mainMgr) {}

    virtual void initializeState();
    virtual void executeState();
    virtual void finalizeState();
    virtual void changeToSubState(const sStateIDIf_c &newState);
    virtual void returnState();
    virtual const sStateIDIf_c *getOldStateID() const;
    virtual void refreshState();
    virtual bool isSubState() const;
    virtual void changeState(const sStateIDIf_c &newState);
    virtual sStateIf_c *getState() const;
    virtual const sStateIDIf_c *getNewStateID() const;
    virtual const sStateIDIf_c *getStateID() const;
    virtual const sStateIDIf_c *getMainStateID() const;

    Manager<T, Method1> mainMgr;
    Manager<T, Method2> subMgr;
    sStateMgrIf_c *currentMgr;
};

#define SSM_T template <class T, template <class, class> class Manager, class Method1, class Method2>
#define SSM_C sStateStateMgr_c<T, Manager, Method1, Method2>

SSM_T void SSM_C::initializeState() { currentMgr->initializeState(); }
SSM_T void SSM_C::executeState() { currentMgr->executeState(); }
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
SSM_T void SSM_C::changeState(const sStateIDIf_c &newState) { currentMgr->changeState(newState); }
SSM_T sStateIf_c *SSM_C::getState() const { return currentMgr->getState(); }
SSM_T const sStateIDIf_c *SSM_C::getNewStateID() const { return currentMgr->getNewStateID(); }
SSM_T const sStateIDIf_c *SSM_C::getStateID() const { return currentMgr->getStateID(); }
SSM_T const sStateIDIf_c *SSM_C::getMainStateID() const { return mainMgr.getStateID(); }

#undef SSM_T
#undef SSM_C
