#include "SPBPriorityQueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>

struct sp_bp_queue_t {
    /* max size of the queue */
    int maxCapacity;
    /* number of elements currently in the queue */
    int numOfElements;
    /* base address of the array of elements */
    BPQueueElement * elementsBasePointer;
};

SPBPQueue* spBPQueueCreate(int maxSize) {
    assert(maxSize > 0);

    SPBPQueue * newSPBPQueue = malloc(sizeof(*newSPBPQueue));
    if (newSPBPQueue == NULL) {
        return NULL;
    }
    int totalSizeOfElements = maxSize * sizeof(*(newSPBPQueue->elementsBasePointer));
    BPQueueElement * newElementsAddress = malloc(totalSizeOfElements);
    if (newElementsAddress == NULL) {
        // we created a new Queue but Create failed. free that queue
        spBPQueueDestroy(newSPBPQueue); 
        return NULL; 
    }
    newSPBPQueue->elementsBasePointer = newElementsAddress;
    newSPBPQueue->maxCapacity = maxSize;
    newSPBPQueue->numOfElements = 0;
    return newSPBPQueue;
}

SPBPQueue* spBPQueueCopy(SPBPQueue* source) {
    assert(source != NULL);

    SPBPQueue * newSPBPQueue = malloc(sizeof(*newSPBPQueue));
    if (newSPBPQueue == NULL) {
        return NULL;
    }

    int totalSize = source->maxCapacity * sizeof(*(source->elementsBasePointer));
    BPQueueElement * newElementsMemory = malloc(totalSize);

    if (newElementsMemory == NULL) {
        /* we allocated for a queue already, but the second malloc failed. free the memory of the queue */
        spBPQueueDestroy(newSPBPQueue); 
        return NULL; 
    }

    memcpy(newElementsMemory, source->elementsBasePointer, totalSize); 
    newSPBPQueue->elementsBasePointer = newElementsMemory ;
    newSPBPQueue->maxCapacity = source->maxCapacity;
    newSPBPQueue->numOfElements = source->numOfElements;
    return newSPBPQueue;

}


void spBPQueueDestroy(SPBPQueue* source) {
    if (source != NULL) {
        free(source->elementsBasePointer);
        free(source);
    }
}

void spBPQueueClear(SPBPQueue* source) {
    assert(source != NULL);
    source->numOfElements = 0;
}

int spBPQueueSize(SPBPQueue* source) {
    assert(source != NULL);
    return source->numOfElements;
}

int spBPQueueGetMaxSize(SPBPQueue* source) {
    assert(source != NULL);
    return source->maxCapacity;
}

SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue* source, int index, double value) {
    if (source == NULL) {
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    if (spBPQueueIsFull(source)) {
        /* don't enqueue if the queue is full and an the value is larger than the max value */
        if (value >= spBPQueueMaxValue(source)) {
            return SP_BPQUEUE_FULL;
        }
        /* remove the largest value element first (by shifting everything one spot to the right, thus overriding the rightmost element which is always the max valued element), and then enqueue as usual */
        else  {
            spBPQueueShiftElements(source, source->numOfElements, SP_BPQUEUE_RIGHT);
            source->numOfElements--;
        }
    }

    /* shift items one spot to the left (to make room for the new item), starting from the first item until shiftUntilIndex */
    int shiftUntilIndex;
    /* start from the first element, and iterate through the array until the value is smaller than the iterated element */
    for (shiftUntilIndex = source->maxCapacity - source->numOfElements; shiftUntilIndex < source->maxCapacity && value >= source->elementsBasePointer[shiftUntilIndex].value ; shiftUntilIndex++);

    /* the number of elements to shift is (shiftUntilIndex - the index of the first item) */
    spBPQueueShiftElements(source, shiftUntilIndex - (source->maxCapacity- source->numOfElements), SP_BPQUEUE_LEFT);

    source->elementsBasePointer[shiftUntilIndex-1].value = value;
    source->elementsBasePointer[shiftUntilIndex-1].index = index;
    source->numOfElements++;
    return SP_BPQUEUE_SUCCESS;
}

void spBPQueueShiftElements(SPBPQueue * source, int numOfElementsToShift, SP_BPQUEUE_DIRECTION direction) {
    int startIndex;
    /* we either shift elements to the right or to the left, depeneding on the direction variable */
    if (direction == SP_BPQUEUE_LEFT) {
        startIndex=source->maxCapacity - source->numOfElements;
        for (int i = startIndex ; i < startIndex + numOfElementsToShift; i++) {
            source->elementsBasePointer[i-1].value = source->elementsBasePointer[i].value;
            source->elementsBasePointer[i-1].index = source->elementsBasePointer[i].index;
        }
    }
    else if (direction == SP_BPQUEUE_RIGHT) {
        startIndex=source->maxCapacity - 1;
        for (int i = startIndex ; i > startIndex - numOfElementsToShift + 1; i--) {
            source->elementsBasePointer[i].value = source->elementsBasePointer[i-1].value;
            source->elementsBasePointer[i].index = source->elementsBasePointer[i-1].index;

        }
    }
}


SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue* source) {
    /* after decrementing numOfElements, the lowest valued element will be overwritten with the next element we enqueue */ 
    if (source == NULL) {
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    if (spBPQueueIsEmpty(source)) {
        return SP_BPQUEUE_EMPTY;
    }
    source->numOfElements -= 1;
    return SP_BPQUEUE_SUCCESS;
}

SP_BPQUEUE_MSG spBPQueuePeek(SPBPQueue* source, BPQueueElement* res) {
    if (source == NULL || res == NULL) {
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    if (source->numOfElements == 0) {
        return SP_BPQUEUE_EMPTY;
    }

    memcpy(res, &(source->elementsBasePointer[source->maxCapacity - source->numOfElements]), sizeof(*res));
    return SP_BPQUEUE_SUCCESS;
}

SP_BPQUEUE_MSG spBPQueuePeekLast(SPBPQueue* source, BPQueueElement* res) {
    if (source == NULL || res == NULL) {
        return SP_BPQUEUE_INVALID_ARGUMENT;
    }
    if (source->numOfElements == 0) {
        return SP_BPQUEUE_EMPTY;
    }

    memcpy(res, &(source->elementsBasePointer[source->maxCapacity - 1]), sizeof(*res));
    return SP_BPQUEUE_SUCCESS;
}

double spBPQueueMinValue(SPBPQueue * source) {
    assert(source != NULL);
    if (spBPQueueIsEmpty(source)) {
        return DBL_MAX;
    }
    return source->elementsBasePointer[source->maxCapacity - source->numOfElements].value;
}
double spBPQueueMaxValue(SPBPQueue * source) {
    assert(source != NULL);
    if (spBPQueueIsEmpty(source)) {
        /* IEEE 754 floating points are symmetrical */
        return -DBL_MAX;
    }
    return source->elementsBasePointer[source->maxCapacity - 1].value;
}

bool spBPQueueIsEmpty(SPBPQueue * source) {
    assert(source != NULL);
    return source->numOfElements == 0;
}

bool spBPQueueIsFull(SPBPQueue * source) {
    assert(source != NULL);
    return source->numOfElements == source->maxCapacity;
}

