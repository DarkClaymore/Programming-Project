#ifndef SPBPRIORITYQUEUE_H_
#define SPBPRIORITYQUEUE_H_
#include <stdbool.h>


/**
 * SP Bounded Priority Queue summary
 * Encapsulates a Bounded Priority Queue with variable capacity.
 * Each element within the queue contains a value of double types, and has a non-negative index.
 *
 *
 * The following functions are supported:
 *
 * spBPQueueCreate - Creates a new queue
 * spBPQueueCopy - Creates a new copy of the given queue
 * spBPQueueDestroy _ Free all resources associated with a queue
 * spBPQueueClear - Clear all elements from the queue
 * spBPQueueSize - Get the number of elements currently in the queue
 * spBPQueueGetMaxSize - Get the queue capacity
 * spBPQueueShiftElements - Used internally by spBPQueueEnqueue. Shifts some elements one spot to the left/right
 * spBPQueueEnqueue - Insert a new element to the queue
 * spBPQueueDequeue - Remove the element with the lowest value from the queue
 * spBPQueuePeek - Returns a copy of the element with the lowest value
 * spBPQueuePeekLast - Returns a copy of the element with the highest value
 * spBPQueueMinValue - Returns the minimum value in the queue
 * spBPQueueMaxValue - Returns the maximum value in the queue
 * spBPQueueIsEmpty - Returns true if the queue is empty
 * spBPQueueIsFull - Returns true if the queue is full
 *
 */

/** type used to define Bounded priority queue **/
typedef struct sp_bp_queue_t SPBPQueue;

typedef struct sp_bpq_element_t {
	int index;
	double value;
} BPQueueElement;

/** type for error reporting **/
typedef enum sp_bp_queue_msg_t {
	SP_BPQUEUE_OUT_OF_MEMORY,
	SP_BPQUEUE_FULL,
	SP_BPQUEUE_EMPTY,
	SP_BPQUEUE_INVALID_ARGUMENT,
	SP_BPQUEUE_SUCCESS
} SP_BPQUEUE_MSG;

typedef enum sp_bp_queue_directions {
    SP_BPQUEUE_LEFT,
    SP_BPQUEUE_RIGHT
} SP_BPQUEUE_DIRECTION;
/**
 * Allocates a new queue in the memory
 * Given the a maxSize variable.
 * Memory is allocated for maxSize possible elements as well. (A total of maxSize * sizeof(BPQueueElement))
 *
 * @param maxSize - the capacity of the queue
 * @assert (maxSize > 0)
 * @return
 * NULL in case memory allocation occurs
 * Otherwise, a pointer to the new queue is returned
 */

SPBPQueue* spBPQueueCreate(int maxSize);

/**
 * Allocates a copy of the given queue.
 *
 * Given the source queue, the function returns a new queue such that:
 * elements(Q) = elements(source) (we allocate new memory and copy the source elements to the new queue)
 * maxCapacity(Q) = maxCapacity(source)
 * numOfElements(Q) = numOfElements(source)
 *
 * @param source - The source queue
 * @assert (source != NULL)
 * @return
 * NULL in case memory allocation occurs
 * Otherwise, a pointer to the copy of queue is returned.
 */

SPBPQueue* spBPQueueCopy(SPBPQueue* source);

/**
 * Free all memory allocation associated with a queue,
 * if source is NULL nothing happens.
 * @param source - The source queue
 */

void spBPQueueDestroy(SPBPQueue* source);

/**
 * Clear all elements from the queue
 * @param source - The source queue
 */
void spBPQueueClear(SPBPQueue* source);

/**
 * A getter for the number of elements currently in the queue
 * @param source - The source queue
 * @return
 * The number of elements in the queue
 */
int spBPQueueSize(SPBPQueue* source);
/**
 * A getter for the queue capacity
 * @param source - The source queue
 * @return
 * The capacity of the queue
 */
int spBPQueueGetMaxSize(SPBPQueue* source);

/**
 * Used internally by spBPQueueEnqueue.
 * Depeneding on the direction, it shifts a number of elements one spot to the left/right starting from the first/last element
 * For example, ShiftLeft([null, null, null, elem1, elem2, elem3], 2, LEFT) -> [null, null, elem1, elem2, null, elem3]
 *
 * @param source - The source queue
 * @param numOfElementsToShift - The number of elements that need to be shifted left
 * @param DIRECTION_TO_SHIFT - Shifts left or right
 *
 * @assert !spBPQueueIsFull(source)
 */
void spBPQueueShiftElements(SPBPQueue * source , int numOfElementsToShift, SP_BPQUEUE_DIRECTION); 

/**
 * Inserts an element to the queue
 * @param source - The source queue
 * @param index - The index of the new element
 * @param value - The value of the new element
 *
 * @return
 * A failure message if the queue is full or the function received an invalid argument
 * Otherwise, a success message
 */

SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue* source, int index, double value);

/**
 * Removes the element with the lowest value from the queue
 * @param source - The source queue
 *
 * @return
 * A failure message if the queue is empty or the function received an invalid argument
 * Otherwise, a success message
 */

SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue* source);

/**
 * Returns a copy of the element with the lowest value
 *
 * @param source - The source queue
 * @param res - a pointer to the memory where the the lowest valued element is copied to
 *
 * @return
 * A failure message if the queue is empty or the function received an invalid argument
 * Otherwise, a success message
 */

SP_BPQUEUE_MSG spBPQueuePeek(SPBPQueue* source, BPQueueElement* res);

/**
 * Returns a copy of the element with the highest value
 *
 * @param source - The source queue
 * @param res - a pointer to the memory where the the highest valued element is copied to
 *
 * @return
 * A failure message if the queue is empty or the function received an invalid argument
 * Otherwise, a success message
 */

SP_BPQUEUE_MSG spBPQueuePeekLast(SPBPQueue* source, BPQueueElement* res);

/**
 * Returns the minimum value in the queue
 *
 * @param source - The source queue
 * @assert source != NULL
 *
 * @return
 * The minimum value in the queue if the queue is not empty
 * Otherwise, INT_MAX
 *
 */
double spBPQueueMinValue(SPBPQueue* source);

/**
 * Returns the maxiumum value in the queue
 *
 * @param source - The source queue
 * @assert source != NULL
 *
 * @return
 * The maximum value in the queue if the queue is not empty
 * Otherwise, INT_MIN
 *
 */

double spBPQueueMaxValue(SPBPQueue* source);

/**
 * Check whether the queue is empty
 * @param souce - The source queue
 * @assert source != NULL
 *
 * @return
 * true if the queue is empty
 * Otherwise, false
 */
bool spBPQueueIsEmpty(SPBPQueue* source);

/**
 * Check whether the queue is full
 * @param souce - The source queue
 * @assert source != NULL
 *
 * @return
 * true if the queue is full
 * Otherwise, false
 */
bool spBPQueueIsFull(SPBPQueue* source);

#endif
