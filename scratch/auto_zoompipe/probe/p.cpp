class B { public: void m(); };
class D : public B {
public:
    unsigned int f;
    char g;
};
class S { public: unsigned int f; char g; };
extern char r1[(((char *)&((D *) 0)->f - (char *) 0) == 0) ? 1 : -1];
extern char r2[(((char *)&((S *) 0)->f - (char *) 0) == 0) ? 1 : -1];
extern char r3[(sizeof(B) == 1) ? 1 : -1];
