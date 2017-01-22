#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include <cstring>

extern "C"{
	#include "SPPoint.h"
}


/*The assumed max length of an image path, as instructed*/
#define MAX_IMG_PATH_LEGTH 1024

/*Input messages*/
#define ENTER_DIRECTORY_MSG "Enter images directory path:\n"
#define ENTER_PREFIX_MSG "Enter images prefix:\n"
#define ENTER_NUM_OF_IMAGES_MSG "Enter number of images:\n"
#define ENTER_SUFFIX_MSG "Enter images suffix:\n"
#define ENTER_NUM_OF_BINS_MSG "Enter number of bins:\n"


/*Exit messages*/
#define MEMORY_ERROR_MSG "An error occurred - allocation failure\n"
#define INVALID_NUM_OF_IMAGES_MSG "An error occurred - invalid number of images\n"
#define INVALID_NUM_OF_BINS_MSG "An error occurred - invalid number of bins\n"
#define INVALID_NUM_OF_FEATURES "An error occurred - invalid number of features\n"
#define EXIT_MSG "Exiting...\n"

/** State machine flags for main(), to trace its state through different sub-methods **/
typedef enum ProgramStateTypes {
	PROGRAM_STATE_RUNNING, /*Main() is still running*/
	PROGRAM_STATE_MEMORY_ERROR, /*Encountered memory allocation error*/
	PROGRAM_STATE_INVALID_N_IMAGES, /*An invalid number of images was inputed*/
	PROGRAM_STATE_INVALID_N_BINS, /*An invalid number of bins was inputed*/
	PROGRAM_STATE_INVALID_N_FEATURES, /*An invalid number of features was inputed*/
	PROGRAM_STATE_EXIT, /*Normal program exit*/
} PROGRAM_STATE;


/*
 * Contains all the info the user inputed for the image database
 */
typedef struct image_database {
	int nImages; /*The number of images*/
	int nBins; /*The number of bins*/
	int nFeaturesToExtract; /*Number of features to extract from each image*/
	char* imgDirectory; /*The directory of the images*/
	char* imgPrefix; /*The prefix of the images*/
	char* imgSuffix; /*The suffix of the images*/
	SPPoint*** RGBHists; /*The RGB histograms of the images*/
} ImageDatabase;




/**
 * Fill the database of images based on the user's input.
 *
 * @param database - pointer to the database to fill.
 *
 * @return:
 * - PROGRAM_STATE_MEMORY_ERROR: Failed to allocate memory at some point.
 * - PROGRAM_STATE_INVALID_N_IMAGES: If num of images < 1
 * - PROGRAM_STATE_INVALID_N_BINS: if num of bins < 1 or > 255
 * - PROGRAM_STATE_INVALID_N_FEATURES: if num of features to extract < 1
 * - PROGRAM_STATE_RUNNING: No errors. Continue running the program.
 */
PROGRAM_STATE GetImageDatabaseFromUser(ImageDatabase* database);

/**
 * Calculates the RGB hists for the database.
 *
 * @param database - pointer to the database to fill.
 * @return
 * - PROGRAM_STATE_MEMORY_ERROR: Failed to allocate memory at some point.
 * - PROGRAM_STATE_RUNNING: No errors. Continue running the program.
 */
PROGRAM_STATE CalcImageDataBaseRGBHists(ImageDatabase* database);

/**
 * Destroy the image database and free all allocated memory for it
 *
 * @param database - pointer to the database to destroy.
 */
void DestroyImageDataBase(ImageDatabase* database);

/**
 * Get the path of a specific image by index.
 *
 * @param imgDirectory - The directory of all images
 * @param imgPrefix - The prefix of the image
 * @param imgSuffix - The suffix of the images
 * @param - The index of the specific image.
 * @return NULL if had memory allocation error. Otherwise, return the path of the image.
 */
char* GetImagePath(char* imgDirectory, char* imgPrefix, char* imgSuffix, int imgIndex);

/**
 * Print the exit message of  main(), depending on its state
 *
 * @param programState - The prgram's state on exit
 */
void PrintExitMessage(PROGRAM_STATE exitProgramState);



#endif /* MAIN_AUX_H_ */
