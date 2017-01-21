#include "SPBPriorityQueue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*A priority bounded queue*/
struct sp_bp_queue_t {
	int maxSize; /*the maximum size of the bounded queue*/
	int size; /*the current size of the queue*/

	/*The queue's elements. Organised in such a way that the lowest value is at the top.*/
	BPQueueElement** elements;
};

SPBPQueue* spBPQueueCreate(int maxSize){
	assert(maxSize > 0); /*Ensure a valid queue size */

	/*Allocate memory for the queue object*/
	SPBPQueue* newBPQueue = (SPBPQueue*)malloc(sizeof(SPBPQueue));
	if (newBPQueue == NULL)
		return NULL; /*Failed to allocate memory*/


	/*Allocate memory for pointer to the elements*/
	BPQueueElement** elements = (BPQueueElement**)malloc(maxSize*sizeof(BPQueueElement*));
	if (elements == NULL)
	{
		free(newBPQueue);  /*Free the allocated memory for the queue class */
		return NULL; /*Failed to allocate memory*/
	}

	/*Set all element pointers to NULL in order to avoid trash pointers*/
	for(int i=0; i<maxSize;i++)
		elements[i]=NULL;

	newBPQueue->elements = elements;
	newBPQueue->maxSize = maxSize;
	newBPQueue->size= 0;
	return newBPQueue;
}

SPBPQueue* spBPQueueCopy(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure that the source is a valid queue*/

	/*Create a new queue with the same max size as the source queue*/
	SPBPQueue* copyQueue = spBPQueueCreate(source->maxSize);

	/*If failed to create due to memory allocation reasons, return null*/
	if (copyQueue == NULL) return NULL;

	/*Stays true if the copy procedure ended in success. Becomes false if failed allocating memory*/
	bool isSuccess = true;

	for(int i=0; i< source->size ;i++)
	{
		/*Allocate memory for the new queue element*/
		copyQueue->elements[i] = (BPQueueElement*)malloc(sizeof(BPQueueElement));

		if (copyQueue->elements[i] == NULL) /*Check if failed to allocate memory*/
		{
			isSuccess = false;
			break;
		}

		/*Copy the content to the newly allocated memory*/
		memcpy(copyQueue->elements[i], source->elements[i], sizeof(BPQueueElement));
		copyQueue->size++;
	}

	/*If failed to do the copying, then free memory and pointer to NULL*/
	if (isSuccess == false)
	{
		spBPQueueDestroy(copyQueue);
		return NULL;
	}

	return copyQueue;
}

//O(1) remove element with the lowest value;
SP_BPQUEUE_MSG spBPQueueDequeue(SPBPQueue* source)
{
	if (source == NULL) /*Make sure that the source is a valid queue*/
		return SP_BPQUEUE_INVALID_ARGUMENT;

	if(spBPQueueIsEmpty(source)) /*Check if the queue is empty*/
		return SP_BPQUEUE_EMPTY;

	free(source->elements[source->size-1]); /*Free the top element in the queue (with lowest value)*/
	source->elements[source->size-1] = NULL; /*Set the pointer to null to avoid trash pointer address*/
	source->size--; /*Decreased queue size by 1*/
	return SP_BPQUEUE_SUCCESS;
}

//O(n)  insert an element to the queue
SP_BPQUEUE_MSG spBPQueueEnqueue(SPBPQueue* source, int index, double value)
{
	if (source == NULL || index < 0) /*Ensure that the source exists and index is valid*/
		return SP_BPQUEUE_INVALID_ARGUMENT;

	/*Check if the queue if full and the new value is too big to be enqueued (bigger than first element)*/
	if (spBPQueueIsFull(source) && value > source->elements[0]->value)
		return SP_BPQUEUE_FULL;

	/*Check if  the queue is full and the new element has the same value as the biggest value in queue, but also has a greater index*/
	if (spBPQueueIsFull(source) && value == source->elements[0]->value && index >= source->elements[0]->index)
		return SP_BPQUEUE_FULL;

	/*Allocate memory for the new element*/
	BPQueueElement* newElement = (BPQueueElement*)malloc(sizeof(BPQueueElement));

	if (newElement == NULL)
		return SP_BPQUEUE_OUT_OF_MEMORY; /*Failed to allocate memory*/

	/*Fill the data for the new element*/
	newElement->index=index;
	newElement->value=value;

	/**
	 * Search for the index in which the new element should be inserted
	* Set the default to insert at the end of the array (will work as intended in case the queue is empty or the new element is smallest in array)
	*/
	int insertIndex = source->size;
	for(int i=0; i < source->size; i++) /*Go over all queued elements and search for the first one with lower value*/
		if (newElement->value >= source->elements[i]->value)
		{
			/*If the value of the new element is equal to an element already in queue, then use index as tie breaker*/
			if (newElement->value == source->elements[i]->value &&
				newElement->index < source->elements[i]->index)
				continue; /*New element has lower index, therefore should go higher in the queue*/

			insertIndex = i; /*Want to insert right where the lower element was and push the rest up*/
			break;
		}


	if (spBPQueueIsFull(source)) /*The queue if full, can't push up*/
	{
		insertIndex--; /*Need to insert right BEFORE the (can't go below 0 due to conditions above) */

		/*The element at the bottom will be pushed out of the queue, so free its memory*/
		free(source->elements[0]);
		source->elements[0] = NULL;

		 /*When the queue is full, then push all everybody below the new element DOWNWARDS*/
		for(int i=0; i < insertIndex; i++)
			/*Overwrite the pointer address of element i with the address of element i+1*/
			source->elements[i] = source->elements[i+1];
	}
	else /*The queue isn't full, can push upwards*/
	{
		for(int i=source->size; i > insertIndex; i--) /*Scan from end until insertion point*/
			/*Overwrite the pointer address of element i with the address of element i-1*/
			source->elements[i] = source->elements[i-1];

		source->size++; /*In this case we increased the queue's size*/
	}

	source->elements[insertIndex] = newElement; /*Point to the new element from the queue*/
	return SP_BPQUEUE_SUCCESS; /*If reached this part, then successfully added the new element*/
}


//O(n)
void spBPQueueDestroy(SPBPQueue* source)
{
	if (source == NULL) return; /*Not pointing to a queue, so no need to clear memory*/

	if (source->elements != NULL) /*If pointing to elements*/
	{
		/*Free memory each element in the queue*/
		for (int j = 0; j < source->size; j++)
		{
			free(source->elements[j]);
		}
	}

	free(source->elements); /*Free the pointer to elements*/
	free(source); /*Free the queue itself*/

}

//O(n)
void spBPQueueClear(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure pointing to a valid source*/

	/*Remove all elements from the queue*/
	/*Loop only needs to run up to the number of elements in queue, not until the end*/
	for (int j = 0; j < source->size; j++)
	{
		free(source->elements[j]); /*Free the memory allocated for this element*/
		source->elements[j] = NULL; /*Make sure that it now points to null and not to some trash address*/
	}

	source->size=0; /*Reset queue's size to 0*/
}

int spBPQueueSize(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure pointing to a valid source*/
	return source->size;
}

int spBPQueueGetMaxSize(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure pointing to a valid source*/
	return source->maxSize;
}



SP_BPQUEUE_MSG spBPQueuePeek(SPBPQueue* source, BPQueueElement* res)
{
	if (source == NULL) /*Check that the source is valid*/
		return SP_BPQUEUE_INVALID_ARGUMENT;

	if(spBPQueueIsEmpty(source)) /*Check if queue is empty*/
		return SP_BPQUEUE_EMPTY;

	/*Copy the element with lowest value in queue and make the result point to the copied memory data*/
	*res = *(source->elements[source->size-1]);
	return SP_BPQUEUE_SUCCESS;
}

SP_BPQUEUE_MSG spBPQueuePeekLast(SPBPQueue* source, BPQueueElement* res){
	if (source == NULL) /*Check that the source is valid*/
		return SP_BPQUEUE_INVALID_ARGUMENT;

	if(spBPQueueIsEmpty(source)) /*Check if queue is empty*/
		return SP_BPQUEUE_EMPTY;

	/*Copy the element with highest value in queue and make the result point to the copied memory data*/

	*res = *(source->elements[0]);
	return SP_BPQUEUE_SUCCESS;
}

//O(1)
double spBPQueueMinValue(SPBPQueue* source)
{
	assert(source != NULL && spBPQueueIsEmpty(source) == false); /*Make sure that the source isn't empty*/

	/*Return the lowest value from the element at the top of the queue*/
	return source->elements[source->size-1]->value;
}

//O(1)
double spBPQueueMaxValue(SPBPQueue* source)
{
	assert(source != NULL && spBPQueueIsEmpty(source) == false); /*Make sure that the source isn't empty*/


	/*Return the highest value from the element at the bottom of the queue*/
	return source->elements[0]->value;
}


//O(1)
bool spBPQueueIsEmpty(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure that the source isn't null*/

	return source->size == 0; /*Return true if no elements in the queue*/
}

//O(1)
bool spBPQueueIsFull(SPBPQueue* source)
{
	assert(source != NULL); /*Make sure that the source isn't null*/

	return source->size == source->maxSize; /*Return true if queue size equals max size*/
}

