// Minimal reproduction probe: does a derived class's implicit deleting
// destructor emit a second, redundant "if (this)" guard before the call to
// an OPAQUE (declared-only) base class virtual destructor?
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    virtual ~Derived();
    static int *m_instance;
};

int *Derived::m_instance;

Derived::~Derived() {
    m_instance = 0;
}
