#include "main_aux.h"
#include "sp_image_proc_util.h"
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>



PROGRAM_STATE GetImageDatabaseFromUser(ImageDatabase* database)
{
	/*Allocate memory*/
	database->imgDirectory = (char*)malloc(sizeof(char) * MAX_IMG_PATH_LEGTH);
	database->imgPrefix = (char*)malloc(sizeof(char) * MAX_IMG_PATH_LEGTH);
	database->imgSuffix = (char*)malloc(sizeof(char) * MAX_IMG_PATH_LEGTH);

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


PROGRAM_STATE CalcImageDataBaseRGBHists(ImageDatabase* database)
{
	/*Create an array of hists*/
	database->RGBHists = (SPPoint***)malloc(sizeof(SPPoint**) * database->nImages);

	if (database->RGBHists == NULL)
		return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	/*Go over each image and calculate the RGB hist*/
	for(int i=0; i < database->nImages; i++)
	{
		/*Get the full path of the image*/
		char* imgPath = GetImagePath(database->imgDirectory, database->imgPrefix, database->imgSuffix, i);

		if (imgPath == NULL)
			return PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

		database->RGBHists[i] = spGetRGBHist(imgPath,i, database->nBins);
		free(imgPath);
	}

	/*All data was calculated successfully. Keep running the main program*/
	return PROGRAM_STATE_RUNNING;
}

char* GetImagePath(char* imgDirectory, char* imgPrefix, char* imgSuffix, int imgIndex)
{
	char* res = (char*)malloc(sizeof(char) * (MAX_IMG_PATH_LEGTH+1));
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


