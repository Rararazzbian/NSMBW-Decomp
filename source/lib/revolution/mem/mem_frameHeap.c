#include <revolution/MEM/mem_frameHeap.h>

#include <revolution/MEM/mem_heapCommon.h>

MEMiHeapHead *MEMDestroyFrmHeap(MEMiHeapHead *heap) {
    MEMiFinalizeHeap(heap);
    return heap;
}
