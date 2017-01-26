#include "main_aux.h"
#include <cstdlib>
/* #include <cstddef> */
#include <cstdio>




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
	printf(ENTER_DIRECTORY_MSG);
	scanf("%s", database->imgDirectory);

	/*Get the image prefix for the images. */
	printf(ENTER_PREFIX_MSG);
	scanf("%s", database->imgPrefix);

	/*Get the number of images*/

	printf(ENTER_NUM_OF_IMAGES_MSG);
	scanf("%d", &database->nImages);

	if (database->nImages < 1) /*Validate the inputed number of images*/
		return PROGRAM_STATE_INVALID_N_IMAGES;

	/*Get the image suffix for the images.*/
	printf(ENTER_SUFFIX_MSG);
	scanf("%s", database->imgSuffix);

	/*Get the number of bins*/
	printf(ENTER_NUM_OF_BINS_MSG);
	scanf("%d", &database->nBins);

	if (database->nBins < 1 || database->nBins > 255) /*Validate the inputed number of bins*/
		return PROGRAM_STATE_INVALID_N_BINS;

	/*Get the number of features to extract from each image*/
	printf("Enter number of features:\n");
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

	/*TODO: Clean for RGBHists*/

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
	for(int i=0; i < database->nImages; i++)
	{
		/*Get the full path of the image*/
		char* imgPath = GetImagePath(database->imgDirectory, database->imgPrefix, database->imgSuffix, i);
		if (imgPath == NULL)
			return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

		/*Calculate RGB hists*/
		database->RGBHists[i] = spGetRGBHist(imgPath,i, database->nBins);
		/*Calculate SIFT descriptors*/
		database->SIFTDescriptors[i] = spGetSiftDescriptors(imgPath,i, database->nFeaturesToExtract, &(database->nFeatures[i]));

		free(imgPath);

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
	int* queryNFeatures = (int*)malloc(sizeof(*queryNFeatures) * 1); /*Num of retrieved features from  query image*/

	/*Allocate memory for the image path for user input*/
	char* queryImagePath = (char*)malloc(sizeof(*queryImagePath) * MAX_IMG_PATH_LEGTH);

	if (queryNFeatures == NULL ||
		queryImagePath == NULL)
		resProgramState = PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
	{
		/*Ask user to input terminating symbol "#" or the path to a query image*/
		printf(ENTER_QUERY_OR_TERMINATE_MSG);
		scanf("%s", queryImagePath);

		if (strcmp(queryImagePath, &TERMINATING_SYMBOL) == 0)
			resProgramState = PROGRAM_STATE_EXIT; /*The user requested to terminate the program*/
	}

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
	{
		queryRGBHists = spGetRGBHist(queryImagePath, QUERY_IMAGE_INDEX, database->nBins);
		querySIFTDescriptors = spGetSiftDescriptors(queryImagePath, QUERY_IMAGE_INDEX, database->nFeaturesToExtract, queryNFeatures);

		if (queryRGBHists == NULL ||
			querySIFTDescriptors == NULL)
			resProgramState = PROGRAM_STATE_MEMORY_ERROR;
	}

	if (resProgramState == PROGRAM_STATE_RUNNING) /*If should keep running or skip to end*/
		/*Calculate and print the indices of closest images based on RGB hists*/
		resProgramState = CalcClosestDatabaseImagesByRGBHists(queryRGBHists, database);

	/* TODO SIFT calculation and printing*/

	/*Free all memory associated with the query image*/
	free(queryImagePath);
	free(queryRGBHists);
	free(querySIFTDescriptors);

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
		for(int i=0; i < database->nImages; i++)
		{
			/*Calculate L2 distance between image i and the query image*/
			double distance = spRGBHistL2Distance(queryRGBHists, database->RGBHists[i]);

			/*Enqueue the L2 distance with the compared image's index*/
			spBPQueueEnqueue(imagesPriortiyQueue, i, distance);
		}

		printf(NEAREST_IMAGES_GLOBAL_DESC_MSG);

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

	/*Destroy the priority queue to free memory*/
	spBPQueueDestroy(imagesPriortiyQueue);

	return PROGRAM_STATE_RUNNING;
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
			printf(MEMORY_ERROR_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_IMAGES:
			printf(INVALID_NUM_OF_IMAGES_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_BINS:
			printf(INVALID_NUM_OF_BINS_MSG);
			break;

		case PROGRAM_STATE_INVALID_N_FEATURES:
			printf(INVALID_NUM_OF_FEATURES);
			break;

		case PROGRAM_STATE_EXIT:
			printf(EXIT_MSG);
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

	for(int i=0; i < *numOfIndices; i++)
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
	for(int i = 0; i < numOfIndices; i++)
	{
		printf("%d",indices[i]); /*Print an index*/

		if (i + 1 < numOfIndices) /*If there are still more indices to print*/
			printf(", ");
	}

	printf("\n"); /*Start new line in the end*/
}


