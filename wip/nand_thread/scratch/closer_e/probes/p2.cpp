// p1 with mMutex-shaped member added: does the presence of a member with its
// own (trivially-inlined) virtual destructor chain change whether the base
// destructor call gets its redundant guard?
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
};

class Derived : public Base {
public:
    virtual ~Derived();
    static int *m_instance;
    Member mMember;
};

int *Derived::m_instance;

Derived::~Derived() {
    m_instance = 0;
}
