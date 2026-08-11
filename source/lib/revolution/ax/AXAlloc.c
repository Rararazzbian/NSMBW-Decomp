#include <revolution/AX/AXAlloc.h>

extern AXVPB *__AXStackHead[AX_PRIORITY_MAX + 1];
extern AXVPB *__AXStackTail[AX_PRIORITY_MAX + 1];
extern AXVPB *__AXCallbackStack;

AXVPB *__AXGetStackHead(u32 prio) {
    return __AXStackHead[prio];
}

void __AXServiceCallbackStack(void) {
    AXVPB *vpb = __AXCallbackStack;

    if (vpb != NULL) {
        __AXCallbackStack = (AXVPB *)vpb->next1;
    }

    while (vpb != NULL) {
        if (vpb->priority != AX_PRIORITY_FREE) {
            if (vpb->callback != NULL) {
                vpb->callback(vpb);
            }

            __AXRemoveFromStack(vpb);

            vpb->next = __AXStackHead[AX_PRIORITY_FREE];
            __AXStackHead[AX_PRIORITY_FREE] = vpb;
            vpb->priority = AX_PRIORITY_FREE;
        }

        vpb = __AXCallbackStack;

        if (vpb != NULL) {
            __AXCallbackStack = (AXVPB *)vpb->next1;
        }
    }
}

void __AXAllocInit(void) {
    int i;

    __AXCallbackStack = NULL;

    for (i = 0; i <= AX_PRIORITY_MAX; i++) {
        __AXStackTail[i] = NULL;
        __AXStackHead[i] = NULL;
    }
}

void __AXPushFreeStack(AXVPB *vpb) {
    vpb->next = __AXStackHead[AX_PRIORITY_FREE];
    __AXStackHead[AX_PRIORITY_FREE] = vpb;
    vpb->priority = AX_PRIORITY_FREE;
}

void __AXPushCallbackStack(AXVPB *vpb) {
    vpb->next1 = __AXCallbackStack;
    __AXCallbackStack = vpb;
}
