struct N { void *a, *b, *c; };   // 0xC
struct A8  { int v[2];  };       // 0x8   %8==0
struct A4  { int v[1];  };       // 0x4   %8==4
struct A10 { int v[4];  };       // 0x10  %8==0
struct A14 { int v[5];  };       // 0x14  %8==4
struct A18 { char v[0x18]; };    // 0x18  %8==0, char array
N n0; A8 a8;
N n1; A4 a4;
N n2; A10 a10;
N n3; A14 a14;
N n4; A18 a18;
