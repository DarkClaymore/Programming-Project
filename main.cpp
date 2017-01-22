#include "sp_image_proc_util.h"
#include "main_aux.h"
#include <cstdlib>
#include <cstddef>
#include <cstdio>

extern "C"{
//include your own C source files
#include "SPPoint.h"
}

int main()
{
	/*Tracks the state of the program*/
	PROGRAM_STATE programState = PROGRAM_STATE_RUNNING;

	/*Initialise and allocate memory for the database of images*/
	ImageDatabase* database = (ImageDatabase*)malloc(sizeof(ImageDatabase));

	/*Fill the database with user's input*/
	programState = GetImageDatabaseFromUser(database);

	/*Calculate RGB histograms for all images*/
	if (programState == PROGRAM_STATE_RUNNING)
		programState = CalcImageDataBaseRGBHists(database);

	/*TODO: stages 8~11 */


	/*Free all memory used by the image database.*/
	DestroyImageDataBase(database);

	/*Print the exit message of the program based on the last program state*/
	PrintExitMessage(programState);

}
