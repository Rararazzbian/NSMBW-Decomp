
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
    Derived::m_instance = 0;
}

