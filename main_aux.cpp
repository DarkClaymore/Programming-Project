#include "main_aux.h"
#include <cstdlib>
#include <cstdio>

extern "C"{
	#include "SPBPriorityQueue.h"
}

const char * TERMINATING_SYMBOL = "#";

PROGRAM_STATE GetImageDatabaseFromUser(ImageDatabase* database)
{
	/*Allocate memory*/
	database->imgDirectory = (char*)malloc(sizeof(*database->imgDirectory) * MAX_IMG_PATH_LEGTH);
	database->imgPrefix = (char*)malloc(sizeof(*database->imgPrefix) * MAX_IMG_PATH_LEGTH);
	database->imgSuffix = (char*)malloc(sizeof(*database->imgSuffix) * MAX_IMG_PATH_LEGTH);

	/*Validate that memory was successfully allocated*/
	if (database->imgDirectory == NULL
		|| database->imgPrefix == NULL
		|| database->imgSuffix == NULL)
		return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	/*Get the directory path for the images.*/
	PrintMsg(ENTER_DIRECTORY_MSG);
	scanf("%s", database->imgDirectory);

	/*Get the image prefix for the images. */
	PrintMsg(ENTER_PREFIX_MSG);
	scanf("%s", database->imgPrefix);

	/*Get the number of images*/

	PrintMsg(ENTER_NUM_OF_IMAGES_MSG);
	scanf("%d", &database->nImages);

	if (database->nImages < 1) /*Validate the inputed number of images*/
		return PROGRAM_STATE_INVALID_N_IMAGES;

	/*Get the image suffix for the images.*/
	PrintMsg(ENTER_SUFFIX_MSG);
	scanf("%s", database->imgSuffix);

	/*Get the number of bins*/
	PrintMsg(ENTER_NUM_OF_BINS_MSG);
	scanf("%d", &database->nBins);

	if (database->nBins < 1 || database->nBins > 255) /*Validate the inputed number of bins*/
		return PROGRAM_STATE_INVALID_N_BINS;

	/*Get the number of features to extract from each image*/
	PrintMsg("Enter number of features:\n");
	scanf("%d", &database->nFeaturesToExtract);

	if (database->nFeaturesToExtract < 1) /*Validate the inputed number of features*/
		return PROGRAM_STATE_INVALID_N_FEATURES;

	/*All data was inputed successfully. Keep running the main program*/
	return PROGRAM_STATE_RUNNING;
}


void DestroyImageDataBase(ImageDatabase* database)
{
	free(database->imgDirectory);
	free(database->imgPrefix);
	free(database->imgSuffix);

	/*Free RGB hists*/
	if (database->nRGBHistsExtracted > 0 && database->RGBHists != NULL)
	{
		for(int i=0; i < database->nRGBHistsExtracted; ++i)
		{
			if (database->RGBHists[i] != NULL)
			{
				for(int j=0; j<NUM_OF_CHANNELS; j++)
					if (database->RGBHists[i][j] != NULL)
						spPointDestroy(database->RGBHists[i][j]);

				free(database->RGBHists[i]);
			}
		}
	}
	free(database->RGBHists);

	/*Free SIFT descriptors*/
	if (database->nSIFTDescriptorsExtracted > 0 &&
		database->SIFTDescriptors != NULL &&
		database->nFeatures != NULL)
	{
		for(int i=0; i < database->nSIFTDescriptorsExtracted; ++i)
		{
			if (database->SIFTDescriptors[i] != NULL)
			{
				for(int j=0; j<database->nFeatures[i]; j++)
					if (database->SIFTDescriptors[i][j] != NULL)
						spPointDestroy(database->SIFTDescriptors[i][j]);
			}
		}
	}
	free(database->nFeatures);
	free(database->SIFTDescriptors);


	free(database); /*Free the database struct itself*/
}


PROGRAM_STATE CalcImageDataBaseHistsAndDescriptors(ImageDatabase* database)
{
	/*Create an array of hists*/
	database->RGBHists = (SPPoint***)malloc(sizeof(*database->RGBHists) * database->nImages);
	database->SIFTDescriptors = (SPPoint***)malloc(sizeof(*database->SIFTDescriptors) * database->nImages);
	database->nFeatures = (int*)malloc(sizeof(*database->nFeatures) * database->nImages);

	if (database->RGBHists == NULL ||
		database->SIFTDescriptors == NULL ||
		database->nFeatures == NULL)
		return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	/*Go over each image and calculate the RGB hist and SIFT descriptors*/
	for(int i=0; i < database->nImages; ++i)
	{
		/*Get the full path of the image*/
		char* imgPath = GetImagePath(database->imgDirectory, database->imgPrefix, database->imgSuffix, i);
		if (imgPath == NULL)
			return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

		/*Calculate RGB hists*/
		database->RGBHists[i] = spGetRGBHist(imgPath,i, database->nBins);

		/*Calculate SIFT descriptors*/
		database->nFeatures[i] = 0; /*Initialise*/
		database->SIFTDescriptors[i] = spGetSiftDescriptors(imgPath,i, database->nFeaturesToExtract, &(database->nFeatures[i]));

		free(imgPath);

		/*Update counters of the number of extracted RGB hists/sift features*/
		if (database->RGBHists[i] != NULL)
			database->nRGBHistsExtracted++;
		if (database->SIFTDescriptors[i] != NULL)
			database->nSIFTDescriptorsExtracted++;

		/*If reached this point in the program, then assume that nBins > 0, maxNFeatures > 0 and image path is valid*/
		/*Therefore, if spGetRGBHist() or SIFTDescriptors() returns null, then it was a memory allocation error*/
		if (database->RGBHists[i] == NULL || database->SIFTDescriptors[i] == NULL)
			return PROGRAM_STATE_MEMORY_ERROR;
	}

	/*All data was calculated successfully. Keep running the main program*/
	return PROGRAM_STATE_RUNNING;
}


PROGRAM_STATE CalcQueryImageClosestDatabaseResults(const ImageDatabase* database)
{
	/*The result of the program's state after this procedure*/
	PROGRAM_STATE resProgramState = PROGRAM_STATE_RUNNING;

	SPPoint** queryRGBHists; /*Query image RGB hists*/
	SPPoint** querySIFTDescriptors; /*Query image descriptors*/

	bool hasQueryRGBHists = false; /*Set to true if the query's RGB hists were successfully retrieved*/
	bool hasQuerySIFTDescriptors = false; /*Set to true if the query's SIFT descriptors were successfully retrieved*/

	int* queryNFeatures = (int*)malloc(sizeof(*queryNFeatures) * 1); /*Num of retrieved features from  query image*/

	/*Allocate memory for the image path for user input*/
	char* queryImagePath = (char*)malloc(sizeof(*queryImagePath) * MAX_IMG_PATH_LEGTH);

	if (queryNFeatures == NULL ||
		queryImagePath == NULL)
		resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
	{
		/*Ask user to input terminating symbol "#" or the path to a query image*/
		PrintMsg(ENTER_QUERY_OR_TERMINATE_MSG);
		scanf("%s", queryImagePath);

		if (strcmp(queryImagePath, TERMINATING_SYMBOL) == 0) {
			resProgramState = PROGRAM_STATE_EXIT; /*The user requested to terminate the program*/
                }
	}

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
	{
		queryRGBHists = spGetRGBHist(queryImagePath, QUERY_IMAGE_INDEX, database->nBins);
		querySIFTDescriptors = spGetSiftDescriptors(queryImagePath, QUERY_IMAGE_INDEX, database->nFeaturesToExtract, queryNFeatures);

		hasQueryRGBHists = queryRGBHists != NULL;
		hasQuerySIFTDescriptors = querySIFTDescriptors != NULL;

		if (queryRGBHists == NULL ||
			querySIFTDescriptors == NULL)
			resProgramState = PROGRAM_STATE_MEMORY_ERROR;
	}

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
		/*Calculate and print the indices of closest images based on RGB hists*/
		resProgramState = CalcClosestDatabaseImagesByRGBHists(queryRGBHists, database);

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
		/*Calculate and print the indices of closest images based on SIFT descriptors*/
		resProgramState = CalcClosestDatabaseImagesBySIFTDescriptors(querySIFTDescriptors, *queryNFeatures, database);

	/*Free all memory associated with the query image*/
	free(queryImagePath);

	if (hasQueryRGBHists) /*If retrieved RGB hists, then destroy all points*/
		for(int i =0; i < NUM_OF_CHANNELS; ++i)
			spPointDestroy(queryRGBHists[i]);
	free(queryRGBHists);

	if (hasQuerySIFTDescriptors) /*If retrieved SIFT features, then destroy all points*/
		for(int i =0; i < *queryNFeatures; ++i)
			spPointDestroy(querySIFTDescriptors[i]);
	free(querySIFTDescriptors);
	free(queryNFeatures);

	return resProgramState;
}


PROGRAM_STATE CalcClosestDatabaseImagesByRGBHists(SPPoint** queryRGBHists, const ImageDatabase* database)
{
	/*The result of the program's state after this procedure*/
	PROGRAM_STATE resProgramState = PROGRAM_STATE_RUNNING;

	/*Use a priority queue to collect the closest images based on L2 distacnes.
	 * The queue doesn't need to be any larger than the number of images to print.*/
	SPBPQueue* imagesPriortiyQueue = spBPQueueCreate(NUM_OF_CLOSEST_IMAGES_TO_PRINT);

	if (imagesPriortiyQueue == NULL)
		resProgramState = PROGRAM_STATE_MEMORY_ERROR;

	if (resProgramState == PROGRAM_STATE_RUNNING)
	{
		for(int i=0; i < database->nImages; ++i)
		{
			/*Calculate L2 distance between image i and the query image*/
			double distance = spRGBHistL2Distance(queryRGBHists, database->RGBHists[i]);

			/*Enqueue the L2 distance with the compared image's index*/
			SP_BPQUEUE_MSG msg = spBPQueueEnqueue(imagesPriortiyQueue, i, distance);

			if (msg == SP_BPQUEUE_OUT_OF_MEMORY)
			{
				resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Memory allocation error in spBPQueueEnqueue()*/
				break;
			}
		}

		if (resProgramState == PROGRAM_STATE_RUNNING)
		{
			PrintMsg(NEAREST_IMAGES_GLOBAL_DESC_MSG);

			int* nearestImgIndices;
			int* numOfIndices = (int*)malloc(sizeof(*numOfIndices) * 1);

			if (numOfIndices == NULL)
				resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Memory allocation error*/

			if (resProgramState == PROGRAM_STATE_RUNNING) /*Continue only if successfully allocated*/
			{
				/*Get the indices of the closest images to the query image*/
				nearestImgIndices = GetBPQueueIndices(imagesPriortiyQueue, numOfIndices);

				/*Print out the indices*/
				PrintIndices(nearestImgIndices, *numOfIndices);
			}

			free(numOfIndices);
			free(nearestImgIndices);
		}
	}

	/*Destroy the priority queue to free memory*/
	spBPQueueDestroy(imagesPriortiyQueue);

	return PROGRAM_STATE_RUNNING;
}

PROGRAM_STATE CalcClosestDatabaseImagesBySIFTDescriptors(SPPoint** querySIFTDescriptors, int nQueryFeatures, const ImageDatabase* database)
{
	/*The result of the program's state after this procedure*/
	PROGRAM_STATE resProgramState = PROGRAM_STATE_RUNNING;

	/*** Counts how many times each image had a descriptor that's close to a descriptor of the query
	  closeDescriptorsCnt[i] = the number of times the i-th image had close descriptors  */
	int* closeDescriptorsCnt = (int*)malloc(sizeof(*closeDescriptorsCnt) * database->nImages);

	/*A priority queue to find the closet images to the query, based on total SIFT feature count*/
	SPBPQueue* imagesPriorityQueue = spBPQueueCreate(NUM_OF_CLOSEST_IMAGES_TO_PRINT);

	if (closeDescriptorsCnt == NULL || imagesPriorityQueue == NULL)
		resProgramState = PROGRAM_STATE_MEMORY_ERROR;

	if (resProgramState == PROGRAM_STATE_RUNNING)
	{
		for(int i=0; i < nQueryFeatures; ++i) /*Go over each feature of the query image*/
		{
			/*The list of the images with closest features to the i-th feature of the quert*/
			int* closetImgIndices;
			closetImgIndices = spBestSIFTL2SquaredDistance(
									NUM_OF_CLOSET_IMAGES_TO_SIFT_FEATURE,
									querySIFTDescriptors[i],
									database->SIFTDescriptors,
									database->nImages,
									database->nFeatures);


			if (closetImgIndices == NULL)
			{
				resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Memory allocation error in spBestSIFTL2SquaredDistance()*/
				break;
			}

			/*Go over the indices of the closet images and add them to the total count*/
			for(int j=0; j<NUM_OF_CLOSET_IMAGES_TO_SIFT_FEATURE;  j++)
			{
				int closetIndex = closetImgIndices[j];
				closeDescriptorsCnt[closetIndex]++;
			}

			free(closetImgIndices); /*Free memory for the list of indices before next iteration*/
		}
	}


	if (resProgramState == PROGRAM_STATE_RUNNING)
	{
		/*Insert the closeness count of each image into the priority queue*/
		for(int i=0; i < database->nImages; ++i)
		{
			/*Since this is a low priority queue, the images will be enqueued with negative count value*/
			/*This way, the images with the highest scores will actually have "lowest priority" in the queue*/
			SP_BPQUEUE_MSG msg = spBPQueueEnqueue(imagesPriorityQueue, i, -closeDescriptorsCnt[i]);

			if (msg == SP_BPQUEUE_OUT_OF_MEMORY)
			{
				resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Memory allocation error in spBPQueueEnqueue()*/
				break;
			}
		}
	}

	if (resProgramState == PROGRAM_STATE_RUNNING)
	{
		int* numOfIndices = (int*)malloc(sizeof(*numOfIndices));
		int* closetImgIndices = GetBPQueueIndices(imagesPriorityQueue, numOfIndices);

		if (closetImgIndices == NULL)
			resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Memory allocation error in GetBPQueueIndices()*/

		if (resProgramState == PROGRAM_STATE_RUNNING)
		{
			PrintMsg(NEAREST_IMAGES_LOCAL_DESC_MSG);
			PrintIndices(closetImgIndices, *numOfIndices);
		}

		free(numOfIndices);
		free(closetImgIndices);
	}

	free(closeDescriptorsCnt);
	spBPQueueDestroy(imagesPriorityQueue);

	return resProgramState;
}




char* GetImagePath(char* imgDirectory, char* imgPrefix, char* imgSuffix, int imgIndex)
{
	char* res = (char*)malloc(sizeof(*res) * (MAX_IMG_PATH_LEGTH+1));
	if (res == NULL)
		return NULL; /*Failed to allocate memory*/

	/*A string representing the imgIndex*/
	/*Assumes max int size of 32,767, which means at most 5 digits*/
	char imgIndexStr[5];
	sprintf(imgIndexStr,"%d",imgIndex);

	/*Construct the path of the image*/
	strcpy(res, imgDirectory);
	strcat(res, imgPrefix);
	strcat(res, imgIndexStr);
	strcat(res, imgSuffix);

	return res;
}

void PrintExitMessage(PROGRAM_STATE exitProgramState)
{
	switch(exitProgramState)
	{
		case PROGRAM_STATE_MEMORY_ERROR:
			PrintMsg(MEMORY_ERROR_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_IMAGES:
			PrintMsg(INVALID_NUM_OF_IMAGES_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_BINS:
			PrintMsg(INVALID_NUM_OF_BINS_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_FEATURES:
			PrintMsg(INVALID_NUM_OF_FEATURES);
			break;

		case PROGRAM_STATE_EXIT:
			PrintMsg(EXIT_MSG);
			break;

		case PROGRAM_STATE_RUNNING:
			break; /*Shouldn't happen. Handled to avoid compilation warnings*/
	}
}


int* GetBPQueueIndices(SPBPQueue* source, int* numOfIndices)
{
	/*Get the size of the priority queue, in case it's lower
	 * than max size of the queue  */
	*numOfIndices = spBPQueueSize(source);

	/*Allocate memory for the output indices*/
	int* resIndices = (int*)malloc((*numOfIndices)*sizeof(*resIndices));

	if (resIndices == NULL)
		return NULL;

	/*Helper queue elem pointer*/
	BPQueueElement* queueElem = (BPQueueElement*)malloc(sizeof(*queueElem));

	for(int i=0; i < *numOfIndices; ++i)
	{
		spBPQueuePeek(source, queueElem); /*Get the first lowest value element*/
		resIndices[i] = queueElem->index; /*Get the index of the element*/
		spBPQueueDequeue(source); /*Remove the element from queue*/
	}

	free(queueElem);

	return resIndices;
}


void PrintIndices(int* indices, int numOfIndices)
{
	/*Print all indices in a i[1], i[2], i[3]... i[n] format, where  n = numOfIndices*/
	for(int i = 0; i < numOfIndices; ++i)
	{
		printf("%d",indices[i]); /*Print an index*/

		if (i + 1 < numOfIndices) /*If there are still more indices to print*/
			PrintMsg(", ");
	}

	PrintMsg("\n"); /*Start new line in the end*/

	fflush(NULL);
}


void PrintMsg(const char* msg)
{
	printf(msg);
	fflush(NULL); /*For windows debugging purposes*/
}


