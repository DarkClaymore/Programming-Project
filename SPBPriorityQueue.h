#ifndef SPBPRIORITYQUEUE_H_
#define SPBPRIORITYQUEUE_H_
#include <stdbool.h>


/** type used to define Bounded priority queue **/
typedef struct sp_bp_queue_t SPBPQueue;

typedef struct sp_bp_element_t {
	int index; /*The index of the element*/
	double value; /*The priority value of the element*/
} BPQueueElement;

/** type for error reporting **/
typedef enum sp_bp_queue_msg_t {
	SP_BPQUEUE_OUT_OF_MEMORY,
	SP_BPQUEUE_FULL,
	SP_BPQUEUE_EMPTY,
	SP_BPQUEUE_INVALID_ARGUMENT,
	SP_BPQUEUE_SUCCESS
} SP_BPQUEUE_MSG;


/**
 * Creates an empty bounded priority queue after allocating memory for it.
 *
 * @param maxSize - the maximum capacity of the
 * @assert (maxSize > 0)
 * @return
 * NULL in case failed to allocate memory.
 * Otherwise, an empty bounded priority queue.
 *
 */
SPBPQueue* spBPQueueCreate(int maxSize);

/**
 * Creates a copy of a given bounded priority queue after allocating memory for it.
 * @param source - the source queue which will be copied.
 * @assert (source != NULL)
 * @return
 * NULL in case failed to allocate memory
 * Otherwise, a copy of the source bounded priority queue
 */
SPBPQueue* spBPQueueCopy(SPBPQueue* source);

/**
 * Frees all memory allocation associated with a bounded priority queue
 * @param source - the pointer to the bounded priority queue
 */
void spBPQueueDestroy(SPBPQueue* source);

/**
 * Removes all the elements in the bounded priority queue]
 * @param source - the pointer to the bounded priority queue
 * @assert (source != NULL)
 */
void spBPQueueClear(SPBPQueue* source);

/**
 * Returns the number of elements in the queue
 * @param source - the pointer to the bounded priority queue
 * @assert (source != NULL)
 * @return
 * The number of elements in the queue
 */
int spBPQueueSize(SPBPQueue* source);

/**
 * Returns the maximum capacity of the queue
 * @param source - the pointer to the bounded priority queue
 * @assert (source != NULL)
 * @return
 * The maximum capacity of the queue
 */
int spBPQueueGetMaxSize(SPBPQueue* source);

/**
 * Inserts an element to the queue, if has place for it.
 * Index is used as a tie breaker between elements of the same value, with lower index placing the element higher in priority
 * (effectively treats its value as "lower")
 * @param source - the pointer to the bounded priority queue
 * @param index - the index of the new element
 * @param value - the value of the new element
 * @return
 * SP_BPQUEUE_FULL: if queue is at maximum size and the value of the new element is bigger than anything else in queue
 * SP_BPQUEUE_INVALID_ARGUMENT: if index <= 0 or source == null
 * SP_BPQUEUE_OUT_OF_MEMORY: if failed to allocate memory for the new queue element
 * SP_BPQUEUE_SUCCESS: if successfully added a new element to the queue
 */
SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue* source, int index, double value);

/**
 * Removes the element with the lowest value
 * @param source - the pointer to the bounded priority queue
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT: if the provided queue pointer is null
 * SP_BPQUEUE_EMPTY: If the queue is empty
 * SP_BPQUEUE_SUCCESS: if successfully took an element out of the queue
 */
SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue* source);

/**
 * Returns a copy of the element with the lowest value in the queue
 * @param source - the pointer to the bounded priority queue
 * @param res - output pointer
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT: if the provided queue pointer is null
 * SP_BPQUEUE_EMPTY: If the queue is empty. Res = NULL in this case.
 * SP_BPQUEUE_SUCCESS: If queue has elements. Res will point to a COPY of the element with the lowest value
 *
 */
SP_BPQUEUE_MSG spBPQueuePeek(SPBPQueue* source, BPQueueElement* res);

/**
 * Returns a copy of the element with the highest value in the queue
 * @param source - the pointer to the bounded priority queue
 * @param res - output pointer
 * @return
 * SP_BPQUEUE_INVALID_ARGUMENT: if the provided queue pointer is null
 * SP_BPQUEUE_EMPTY: If the queue is empty. Res = NULL in this case.
 * SP_BPQUEUE_SUCCESS: If queue has elements. Res will point to a COPY of the element with the highest value
 *
 */
SP_BPQUEUE_MSG spBPQueuePeekLast(SPBPQueue* source, BPQueueElement* res);

/**
 * Returns the minimum value in the queue
 * @param source - the pointer to the bounded priority queue
 * @assert (source != null && spBPQueueIsEmpty(source) == false)
 * @return
 * /The minimum value in the queue
 */
double spBPQueueMinValue(SPBPQueue* source);

/**
 * Returns the maximum value in the queue
 * @param source - the pointer to the bounded priority queue
 * @assert (source != null && spBPQueueIsEmpty(source) == false)
 * @return
 * /The maximum value in the queue
 */
double spBPQueueMaxValue(SPBPQueue* source);

/**
 * Checks if the queue is empty
 * @assert (source != null)
 * @return
 * TRUE if the queue is empty. FALSE otherwise.
 */
bool spBPQueueIsEmpty(SPBPQueue* source);

/**
 * Checks if the queue is full
 * @assert (source != null)
 * @return
 * TRUE if the queue is full. FALSE otherwise.
 */
bool spBPQueueIsFull(SPBPQueue* source);

#endif
