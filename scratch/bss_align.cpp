struct Node { void *a, *b, *c; };          // 0xC, align 4
struct S98 { int v[0x98/4]; };             // 0x98, align 4, size %8 == 0
struct S5C { int v[0x5C/4]; };             // 0x5C, align 4, size %8 == 4
struct S58 { int v[0x58/4]; };             // 0x58, align 4, size %8 == 0
struct SC5C { int v[0xC5C/4]; };           // 0xC5C, align 4, size %8 == 4
Node n0; S98 o98;
Node n1; S5C o5c;
Node n2; S58 o58;
Node n3; SC5C oc5c;
