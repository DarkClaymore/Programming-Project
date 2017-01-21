#include <stddef.h>
#include <stdlib.h>

// Added by GUY
#include "sp_image_proc_util.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace cv;

extern "C"{
	//Use this syntax in-order to include C-header files
	//HINT : You don't need to include other C header files here -> Maybe in sp_image_proc_util.c ? <-
        //
        // Commented out by GUY - SPPoint.h is already included in sp_image_proc_util.h
	/* #include "SPPoint.h" */
	#include "SPBPriorityQueue.h"
}

/*The number of channels that are expected on input (R,G,B)*/
const int NUM_OF_CHANNELS = 3; 

/* Descriptor dimension */
const int SIFT_DESCRIPTOR_DIM = 128; 


SPPoint** spGetRGBHist(const char* str,int imageIndex, int nBins) {
    if (str == NULL || nBins <= 0) {
        return NULL;
    }
    SPPoint ** pointsArray;
    Mat src;

    /* Load image */
    src = imread(str, CV_LOAD_IMAGE_COLOR);

    if (src.empty()) {
        return NULL;
    }

    /* Separate the image in 3 places ( B, G and R ) */
    std::vector<Mat> bgr_planes;
    split(src, bgr_planes);

    /* Set the ranges ( for B,G,R) */
    float range[] = { 0, 256 };
    const float* histRange = { range };

    /* Set the other parameters: */
    int nImages = 1;

    /* red histogram, green histogram, blue histogram */
    Mat hists [NUM_OF_CHANNELS];

    /* allocate our point pointers array (to store histograms in) */
    pointsArray = (SPPoint**)malloc(sizeof(*pointsArray) * NUM_OF_CHANNELS);
    if (pointsArray == NULL) {
        return NULL;
    }

    /* Compute the histograms, and store their data in a point */ 
    /* The output type of the matrices is CV_32F (float), we cast it to double */
    bool pointsCreateFailure = false;
    for (int i = 0; i < NUM_OF_CHANNELS; i++) {
        calcHist(&bgr_planes[i], nImages, 0, Mat(), hists[i], 1, &nBins, &histRange);
        /* flip BGR to RGB */
        /* TODO: make sure we can cast this way */
        pointsArray[NUM_OF_CHANNELS - i - 1] = spPointCreate((double*)hists[i].data, nBins, imageIndex);
        if (pointsArray[NUM_OF_CHANNELS - i - 1] == NULL) {
            pointsCreateFailure = true;
        }
    }
    if (pointsCreateFailure) {
        for (int i = 0; i < NUM_OF_CHANNELS; i++) {
            spPointDestroy(pointsArray[i]); 
        }
        free(pointsArray);
        pointsArray = NULL;
    }
    return pointsArray;
}

double spRGBHistL2Distance(SPPoint** rgbHistA, SPPoint** rgbHistB) {
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

SPPoint ** spGetSiftDescriptors(char* str, int imageIndex, int nFeaturesToExtract, int *nFeatures) {
    if (str == NULL || nFeaturesToExtract <= 0 || nFeatures == NULL) {
        return NULL;
    }
    SPPoint ** pointsArray;
    cv::Mat src;

    /* Load img - gray scale mode! */
    src = cv::imread(str, CV_LOAD_IMAGE_GRAYSCALE);
    if (src.empty()) {
        return NULL;
    }

    /* Key points will be stored in kp1; */
    std::vector<cv::KeyPoint> kp1;

    /* Feature values will be stored in ds1; */
    cv::Mat ds1;

    /* Creating  a Sift Descriptor extractor */
    cv::Ptr<cv::xfeatures2d::SiftDescriptorExtractor> detect =
        cv::xfeatures2d::SIFT::create(nFeaturesToExtract);

    /* Extracting features */
    /* The features will be stored in ds1 */
    /* The output type of ds1 is CV_32F (float) */
    detect->detect(src, kp1, cv::Mat());
    detect->compute(src, kp1, ds1);
    if (ds1.empty()) {
        return NULL;
    }

    /* number of features we actually extracted */
    *nFeatures = ds1.size().height;

    /* allocate nFeatures points */
    pointsArray = (SPPoint**)malloc(sizeof(*pointsArray) * *nFeatures);
    if (pointsArray == NULL) {
        return NULL;
    }

    /* create an array of nFeatures points, each containing the corresponding descriptor */
    bool pointsCreateFailure = false;
    for (int i = 0; i < *nFeatures; i++) {
        // TODO: not sure about that, debug!
        pointsArray[i] = spPointCreate((double*)ds1.row(i).data, SIFT_DESCRIPTOR_DIM, imageIndex);            
        if (pointsArray[i] == NULL) {
            pointsCreateFailure = true;
        }
    }
    // TODO: move this one to external function?
    if (pointsCreateFailure) {
        for (int i = 0; i < *nFeatures; i++) {
            spPointDestroy(pointsArray[i]); 
        }
        free(pointsArray);
        pointsArray = NULL;
    }
    return 0;

}

int* spBestSIFTL2SquaredDistance(int kClosest, SPPoint* queryFeature, SPPoint*** databaseFeatures, int numberOfImages, int* nFeaturesPerImage) {

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
            // GUY - [i] is a mistake?
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
