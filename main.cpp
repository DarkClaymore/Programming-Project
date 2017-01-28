#include "main_aux.h"
#include <cstdlib>


int main()
{
	/*Tracks the state of the program*/
	PROGRAM_STATE programState = PROGRAM_STATE_RUNNING;

	/*Initialise and allocate memory for the database of images*/
	ImageDatabase* database = (ImageDatabase*)malloc(sizeof(*database));

	if (database == NULL)
		programState = PROGRAM_STATE_MEMORY_ERROR; /*Failed to allocate memory*/

	/*Initialise int values to not hold garbage data in later stages if not overwritten by input*/
	database->nImages = 0;
	database->nBins = 0;
	database->nFeaturesToExtract = 0;
	database->nRGBHistsExtracted = 0;
	database->nSIFTDescriptorsExtracted = 0;


	/*Fill the database with user's input*/
	programState = GetImageDatabaseFromUser(database);

	/*Calculate RGB histograms for all images*/
	if (programState == PROGRAM_STATE_RUNNING)
		programState = CalcImageDataBaseHistsAndDescriptors(database);

	/*While the user hasn't entered the exit symbol or an error hasn't occurred, receive query image inputs*/
	while (programState == PROGRAM_STATE_RUNNING)
		programState = CalcQueryImageClosestDatabaseResults(database);

	/*Free all memory used by the image database.*/
	DestroyImageDataBase(database);

	/*Print the exit message of the program based on the last program state*/
	PrintExitMessage(programState);
	return 0;
}
