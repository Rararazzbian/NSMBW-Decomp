#include <revolution/AX/AXAlloc.h>
#include <revolution/OS/OSInterrupt.h>

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

void __AXRemoveFromStack(AXVPB *vpb) {
    u32 prio = vpb->priority;
    AXVPB *head = __AXStackHead[prio];
    AXVPB *tail = __AXStackTail[prio];

    if (head == tail) {
        __AXStackTail[prio] = NULL;
        __AXStackHead[prio] = NULL;
        return;
    }

    if (vpb == head) {
        AXVPB *next = (AXVPB *)vpb->next;

        __AXStackHead[prio] = next;
        next->prev = NULL;
        return;
    }

    if (vpb == tail) {
        AXVPB *prev = (AXVPB *)vpb->prev;

        __AXStackTail[prio] = prev;
        prev->next = NULL;
        return;
    }

    {
        AXVPB *prev = (AXVPB *)vpb->prev;
        AXVPB *next = (AXVPB *)vpb->next;

        prev->next = next;
        next->prev = prev;
    }
}

void AXFreeVoice(AXVPB *vpb) {
    BOOL old = OSDisableInterrupts();

    __AXRemoveFromStack(vpb);

    if (vpb->pb.state == AX_VOICE_RUN) {
        vpb->depop = 1;
    }

    __AXSetPBDefault(vpb);

    vpb->next = __AXStackHead[AX_PRIORITY_FREE];
    __AXStackHead[AX_PRIORITY_FREE] = vpb;
    vpb->priority = AX_PRIORITY_FREE;

    OSRestoreInterrupts(old);
}
