#include <stddef.h>
#include <stdlib.h>

extern "C"{
	//Use this syntax in-order to include C-header files
	//HINT : You don't need to include other C header files here -> Maybe in sp_image_proc_util.c ? <-
	#include "SPPoint.h"
	#include "SPBPriorityQueue.h"
}

/*The number of channels that are expected on input (R,G,B)*/
#define NUM_OF_CHANNELS 3


double spRGBHistL2Distance(SPPoint** rgbHistA, SPPoint** rgbHistB)
{
	double averageDistance = 0;

	//Go over each channel and calculate the L2 distance between hist vectors in A and B
	//Add all distances multiplied by 0.33 to get the average distance
	for(int i=0; i < NUM_OF_CHANNELS; i++)
	{
		if (rgbHistA+i == NULL || rgbHistB+i == NULL)
			return -1; //A null pointer in one of the channels. Distance can't be calculated.

		averageDistance += 0.33 * spPointL2SquaredDistance(rgbHistA[i], rgbHistB[i]);
	}

	return averageDistance;
}



int* spBestSIFTL2SquaredDistance(int kClosest, SPPoint* queryFeature,
		SPPoint*** databaseFeatures, int numberOfImages,
		int* nFeaturesPerImage)
{
	/*Input validation*/
	if (queryFeature == NULL ||
		databaseFeatures == NULL ||
		numberOfImages <= 1 ||
		nFeaturesPerImage == NULL)
		return NULL;


	/*Allocate memory for storing the kClosest indices of images closest to queryFeature*/
	int* closestImgIndices = (int*)malloc(kClosest*sizeof(int));
	if (closestImgIndices == NULL)
		return NULL; /*Failed to allocate memory*/

	/*Create a priority queue of size kClosest*/
	SPBPQueue* priorityQueue = spBPQueueCreate(kClosest);

	for(int i = 0; i < numberOfImages; i++) /*Go over each image*/
		for(int j = 0; j < *nFeaturesPerImage[i]; j++) /*Go over each feature in image*/
		{
			/*Calculate the L2 distance between the feature and queryFeature*/
			double distance = spPointL2SquaredDistance(databaseFeatures[i][j], queryFeature);

			/*Enqueue the L2 distance to the priority queue, with the image's index*/
			spBPQueueEnqueue(priorityQueue, i, distance);
		}

	/*Get the size of the queue, in case it's smaller than kClosest*/
	int queueSize = spBPQueueSize(priorityQueue);
	BPQueueElement queueElem; /*A helper to retrieve elements from the priority queue*/

	/*Go over all elements in the priority queue to retrieve image indices*/
	for(int i = 0; i < queueSize; i++)
	{
		/*Get the element with the lowest value in the queue*/
		spBPQueuePeek(priorityQueue, &queueElem);

		/*Add the index of the queue element to the output array*/
		closestImgIndices[i] = queueElem.index;

		/*Dequeue the lowest value element in preparation for the next iteration*/
		spBPQueueDequeue(priorityQueue);
	}

	/*Destroy the priority queue to free all its memory*/
	spBPQueueDestroy(priorityQueue);

	return closestImgIndices;
}
