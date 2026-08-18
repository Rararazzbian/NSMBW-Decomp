
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    virtual ~Derived();
    static void clearInstance() { m_instance = 0; }
    static int *m_instance;
};

int *Derived::m_instance;

Derived::~Derived() {
    clearInstance();
}
