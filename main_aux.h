#ifndef MAIN_AUX_H_
#define MAIN_AUX_H_

#include <cstring>
#include "sp_image_proc_util.h"

extern "C"{
	#include "SPBPriorityQueue.h"
}


const char TERMINATING_SYMBOL = '#';

/*The assumed max length of an image path, as instructed*/
#define MAX_IMG_PATH_LEGTH 1024

/*The index that a query image will receive in SPPoint objects*/
#define QUERY_IMAGE_INDEX 1

/*The number of closest images to print for an inputed query image ,
 * Applies for both RGB hist distance compare and to SIFT descriptors distance*/
#define NUM_OF_CLOSEST_IMAGES_TO_PRINT 5

/* The number of closet images to count for each SIFT feature of the query image
 */
#define NUM_OF_CLOSET_IMAGES_TO_SIFT_FEATURE 5

/*The number of channels that are expected on input (R,G,B)*/
#define NUM_OF_CHANNELS 3


/*Input messages*/
#define ENTER_DIRECTORY_MSG "Enter images directory path:\n"
#define ENTER_PREFIX_MSG "Enter images prefix:\n"
#define ENTER_NUM_OF_IMAGES_MSG "Enter number of images:\n"
#define ENTER_SUFFIX_MSG "Enter images suffix:\n"
#define ENTER_NUM_OF_BINS_MSG "Enter number of bins:\n"
#define ENTER_QUERY_OR_TERMINATE_MSG "Enter a query image or # to terminate:\n"
#define NEAREST_IMAGES_GLOBAL_DESC_MSG "Nearest images using global descriptors:\n"

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
	int nBins; /*The number of bins to e*/
	int nFeaturesToExtract; /*Number of features to extract from each image*/

	int nRGBHistsExtracted; /*The number of images for which RGB hists were successfully extracted*/
	int nSIFTDescriptorsExtracted; /*The number of images for which sift features were successfully extracted*/

	char* imgDirectory; /*The directory of the images*/
	char* imgPrefix; /*The prefix of the images*/
	char* imgSuffix; /*The suffix of the images*/
	SPPoint*** RGBHists; /*The RGB histograms of the images*/
	SPPoint*** SIFTDescriptors; /*The SIFT descriptors of the images*/
	int* nFeatures; /*The actual number of features that was extracted for each image*/
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
 * Calculates the RGB hists and SIFT descriptors for the database.
 *
 * @param database - pointer to the database to fill.
 * @return
 * - PROGRAM_STATE_MEMORY_ERROR: Failed to allocate memory at some point.
 * - PROGRAM_STATE_RUNNING: No errors. Continue running the program.
 */
PROGRAM_STATE CalcImageDataBaseHistsAndDescriptors(ImageDatabase* database);

/**
 * Lets user input the relative URL of a query image.
 * Calculates the closest images in database to the query image, based
 * on RGB hists and SIFT descriptors.
 *
 * @param database - the database of images.
 * @return
 * - PROGRAM_STATE_EXIT: User inputed an exit symbol "#" to terminate the program.
 * - PROGRAM_STATE_MEMORY_ERROR: Failed to allocate memory at some point.
 * - PROGRAM_STATE_RUNNING: Closest images were successfully calculated and printed.
 * 							Keep running program for another user input.
 */
PROGRAM_STATE CalcQueryImageClosestDatabaseResults(const ImageDatabase* database);

/***
 * Calculates and prints the closest NUM_OF_CLOSEST_IMAGES_TO_PRINT images to the query image
 * based on L2 distances of RGB hists.
 *
 * @param queryRGBHists - the RGB hists of the query image.
 * @param database - the database of images with which the query image will be compared.
 *
 */
PROGRAM_STATE CalcClosestDatabaseImagesByRGBHists(SPPoint** queryRGBHists, const ImageDatabase* database);


/***
 * Calculates and prints the closest NUM_OF_CLOSEST_IMAGES_TO_PRINT images to the query image
 * based on L2 distances of SIFT descriptors.
 *
 * The closest images will be the ones which have the highest total number of closest
 * SIFT descriptors
 *
 * @param querySIFTDescriptors - all the SIFT descriptors of the query.
 * @param nQueryFeatures - the number of SIFT descriptors the query has
 * @param database - the database of images with which the query image will be compared.
 */
PROGRAM_STATE CalcClosestDatabaseImagesBySIFTDescriptors(SPPoint** querySIFTDescriptors, int nQueryFeatures, const ImageDatabase* database);


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


/**
 * Dequeue all queue elements and return their indices in order (from lowest value elem to highest).
 * Assumes that the caller will handle the destruction of the queue and freeing memory.
 *
 * @param source - the BP queue to get the indices of.
 * @param numOfIndices - OUTPUT parameter. The number of indices in the result.
 * @return
 * - NULL if memory allocation error happened.
 * - Otherwise, the indices of all the queue elements, in order from lowest value elem to highest value elem
 */
int* GetBPQueueIndices(SPBPQueue* source, int* numOfIndices);

/***
 * Prints an array of provided indices in a specific format.
 *
 * @param indices - the array of indices
 * @param numOfIndices - the size of indices array
 */
void PrintIndices(int* indices, int numOfIndices);

/***
 * A wrapper for a printf method that prints a static message.
 * Used in order turn on or off fflush(NULL) for windows debugging purposes
 *
 * @param msg - the msg to print
 */
void PrintMsg(const char* msg);



#endif /* MAIN_AUX_H_ */
