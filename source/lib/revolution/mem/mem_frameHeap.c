#include <revolution/MEM/mem_frameHeap.h>

#include <revolution/MEM/mem_heapCommon.h>

#define FRM_HEAP_MAGIC 0x46524D48 /* 'FRMH' */

#define ROUND_DOWN(value, align) ((u32)(value) & ~((align) - 1))

static MEMiFrmHeapHead *GetFrmHeapHeadPtrFromHeapHead(MEMiHeapHead *heap) {
    return (MEMiFrmHeapHead *)AddU32ToPtr(heap, sizeof(MEMiHeapHead));
}

MEMiHeapHead *MEMCreateFrmHeapEx(void *start, u32 size, u16 opt) {
    void *end = AddU32ToPtr(start, size);
    void *heapEnd = (void *)ROUND_DOWN(end, 4);
    MEMiHeapHead *heap = (MEMiHeapHead *)ROUND_UP((u32)start, 4);
    MEMiFrmHeapHead *frmHeap;

    if ((u32)heap > (u32)heapEnd ||
        (u32)GetOffsetFromPtr(heap, heapEnd) < MEM_FRM_HEAP_HEAD_SIZE) {
        return NULL;
    }

    MEMiInitHeapHead(heap, FRM_HEAP_MAGIC,
                     AddU32ToPtr(heap, MEM_FRM_HEAP_HEAD_SIZE), heapEnd, opt);

    frmHeap = GetFrmHeapHeadPtrFromHeapHead(heap);
    frmHeap->head = heap->start;
    frmHeap->tail = heap->end;
    frmHeap->states = NULL;

    return heap;
}

MEMiHeapHead *MEMDestroyFrmHeap(MEMiHeapHead *heap) {
    MEMiFinalizeHeap(heap);
    return heap;
}
