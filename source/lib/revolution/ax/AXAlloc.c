#include <revolution/AX/AXAlloc.h>

extern AXVPB *__AXStackHead[AX_PRIORITY_MAX + 1];

AXVPB *__AXGetStackHead(u32 prio) {
    return __AXStackHead[prio];
}
