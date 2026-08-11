#include <revolution/OS/OSArena.h>

extern void *__OSArenaHi;
extern void *__OSArenaLo;
extern void *s_mem2ArenaHi;
extern void *s_mem2ArenaLo;

void *OSGetMEM1ArenaHi(void) { return __OSArenaHi; }
void *OSGetMEM2ArenaHi(void) { return s_mem2ArenaHi; }
void *OSGetArenaHi(void) { return __OSArenaHi; }

void *OSGetMEM1ArenaLo(void) { return __OSArenaLo; }
void *OSGetMEM2ArenaLo(void) { return s_mem2ArenaLo; }
void *OSGetArenaLo(void) { return __OSArenaLo; }

void OSSetMEM1ArenaHi(void *hi) { __OSArenaHi = hi; }
void OSSetMEM2ArenaHi(void *hi) { s_mem2ArenaHi = hi; }
void OSSetArenaHi(void *hi) { __OSArenaHi = hi; }

void OSSetMEM1ArenaLo(void *lo) { __OSArenaLo = lo; }
void OSSetMEM2ArenaLo(void *lo) { s_mem2ArenaLo = lo; }
void OSSetArenaLo(void *lo) { __OSArenaLo = lo; }
