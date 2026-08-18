
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    ~Derived();
    static int *m_instance;
};

int *Derived::m_instance;

Derived::~Derived() {
    m_instance = 0;
}

