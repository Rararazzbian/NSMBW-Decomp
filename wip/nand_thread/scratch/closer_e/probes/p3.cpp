// p2 plus: base class has 3 more virtuals (dtor, run, onEnter, onExit) like
// EGG::Thread, and Derived overrides run() too (like dNandThread_c does).
// Does having MULTIPLE virtual slots / an overridden non-dtor virtual change
// whether the base dtor call gets its redundant guard?
class MutexBase {
public:
    virtual ~MutexBase() {}
};

class Member : public MutexBase {
public:
    virtual ~Member() {}
};

class Base {
public:
    virtual ~Base();
    virtual void *run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}
    unsigned char mPad[0x48];
};

class Derived : public Base {
public:
    virtual ~Derived();
    virtual void *run();
    static int *m_instance;
    Member mMember;
};

int *Derived::m_instance;

Derived::~Derived() {
    m_instance = 0;
}
