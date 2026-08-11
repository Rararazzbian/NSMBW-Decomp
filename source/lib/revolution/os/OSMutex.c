#include <revolution/OS/OSMutex.h>

#include <revolution/OS/OSThread.h>

/**
 * Adds @p mutex to the tail of the mutex queue owned by @p thread.
 */
#define ENQUEUE_MUTEX_TAIL(thread, mutex)                                      \
    do {                                                                       \
        OSMutex *__prev = (thread)->mutexQueue.tail;                           \
        if (__prev == NULL) {                                                  \
            (thread)->mutexQueue.head = (mutex);                               \
        } else {                                                               \
            __prev->next = (mutex);                                            \
        }                                                                      \
        (mutex)->prev = __prev;                                                \
        (mutex)->next = NULL;                                                  \
        (thread)->mutexQueue.tail = (mutex);                                   \
    } while (0)

/**
 * Removes @p mutex from the mutex queue owned by @p thread.
 */
#define DEQUEUE_MUTEX(thread, mutex)                                           \
    do {                                                                       \
        OSMutex *__next = (mutex)->next;                                       \
        OSMutex *__prev = (mutex)->prev;                                       \
        if (__next == NULL) {                                                  \
            (thread)->mutexQueue.tail = __prev;                                \
        } else {                                                               \
            __next->prev = __prev;                                             \
        }                                                                      \
        if (__prev == NULL) {                                                  \
            (thread)->mutexQueue.head = __next;                                \
        } else {                                                               \
            __prev->next = __next;                                             \
        }                                                                      \
    } while (0)

void OSInitMutex(OSMutex *mutex) {
    OSInitThreadQueue(&mutex->queue);
    mutex->thread = NULL;
    mutex->lock = 0;
}

void OSLockMutex(OSMutex *mutex) {
    BOOL enabled = OSDisableInterrupts();
    OSThread *currentThread = OSGetCurrentThread();
    OSThread *owner;

    while (TRUE) {
        owner = mutex->thread;
        if (owner == NULL) {
            mutex->thread = currentThread;
            mutex->lock++;
            ENQUEUE_MUTEX_TAIL(currentThread, mutex);
            break;
        }

        if (owner == currentThread) {
            mutex->lock++;
            break;
        }

        currentThread->mutex = mutex;
        __OSPromoteThread(mutex->thread, currentThread->priority);
        OSSleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    OSRestoreInterrupts(enabled);
}

void OSUnlockMutex(OSMutex *mutex) {
    BOOL enabled = OSDisableInterrupts();
    OSThread *currentThread = OSGetCurrentThread();

    if (mutex->thread == currentThread && --mutex->lock == 0) {
        DEQUEUE_MUTEX(currentThread, mutex);
        mutex->thread = NULL;

        if (currentThread->priority < currentThread->base) {
            currentThread->priority = __OSGetEffectivePriority(currentThread);
        }

        OSWakeupThread(&mutex->queue);
    }

    OSRestoreInterrupts(enabled);
}

void __OSUnlockAllMutex(OSThread *thread) {
    OSMutex *mutex;

    while ((mutex = thread->mutexQueue.head) != NULL) {
        OSMutex *next = mutex->next;
        if (next == NULL) {
            thread->mutexQueue.tail = NULL;
        } else {
            next->prev = NULL;
        }
        thread->mutexQueue.head = next;

        mutex->lock = 0;
        mutex->thread = NULL;
        OSWakeupThread(&mutex->queue);
    }
}

BOOL OSTryLockMutex(OSMutex *mutex) {
    BOOL enabled = OSDisableInterrupts();
    OSThread *currentThread = OSGetCurrentThread();
    BOOL locked;

    if (mutex->thread == NULL) {
        mutex->thread = currentThread;
        mutex->lock++;
        ENQUEUE_MUTEX_TAIL(currentThread, mutex);
        locked = TRUE;
    } else if (mutex->thread == currentThread) {
        mutex->lock++;
        locked = TRUE;
    } else {
        locked = FALSE;
    }

    OSRestoreInterrupts(enabled);
    return locked;
}
