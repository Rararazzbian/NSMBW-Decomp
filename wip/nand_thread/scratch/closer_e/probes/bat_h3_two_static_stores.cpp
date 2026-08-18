
class Base {
public:
    virtual ~Base();
};

class Derived : public Base {
public:
    virtual ~Derived();
    static int *m_instance;
    static int s_flag;
};

int *Derived::m_instance;
int Derived::s_flag;

Derived::~Derived() {
    m_instance = 0;
    s_flag = 0;
}
