#include "sp_image_proc_util.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace cv;

extern "C"{
	#include "SPBPriorityQueue.h"
}

/*The number of channels that are expected on input (R,G,B)*/
#define NUM_OF_CHANNELS 3

/* Error message in case we loaded an empty image */
#define EMPTY_IMAGE_LOADED_ERROR "Image cannot be loaded"
#define EMPTY_IMAGE_LOADED_ERROR_FORMAT "%s - %s:\n"

/* Error code for exiting after an empty image is tried to be loaded*/
#define ERROR_CODE -1

/*The proportion each channel contributes toward the total L2 distance calculated based on RGB hists*/
#define CHANNEL_L2_DISTANCE_PROPORTION 0.33


SPPoint** spGetRGBHist(const char* str,int imageIndex, int nBins) {
    if (str == NULL || nBins <= 0) {
        return NULL;
    }
    Mat src;

    /* Load image */
    src = imread(str, CV_LOAD_IMAGE_COLOR);

    if (src.empty()) {
        printf(EMPTY_IMAGE_LOADED_ERROR_FORMAT,EMPTY_IMAGE_LOADED_ERROR, str);
        exit(ERROR_CODE);

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
    SPPoint ** pointsArray = (SPPoint**)malloc(sizeof(*pointsArray) * NUM_OF_CHANNELS);
    if (pointsArray == NULL) {
        return NULL;
    }

    /* Compute the histograms, and store their data in a point */ 
    /* The output type of the matrices is CV_32F (float), we cast it to double */
    bool pointsCreateFailure = false;
    double * histData = NULL;
    histData = (double*)malloc(sizeof(*histData) * nBins);
    if (histData == NULL) {
        pointsCreateFailure = true;
    }
    for (int i = 0; i < NUM_OF_CHANNELS && !pointsCreateFailure; ++i) {
        calcHist(&bgr_planes[i], nImages, 0, Mat(), hists[i], 1, &nBins, &histRange);
        for (int j = 0; j < nBins; ++j) {
           histData[j] = hists[i].at<float>(j);
        }
        /* flip BGR to RGB */
        pointsArray[NUM_OF_CHANNELS - i - 1] = spPointCreate(histData, nBins, imageIndex);
        if (pointsArray[NUM_OF_CHANNELS - i - 1] == NULL) {
            pointsCreateFailure = true;
        }
    }
    free(histData);
    if (pointsCreateFailure) {
        for (int i = 0; i < NUM_OF_CHANNELS; ++i) {
            spPointDestroy(pointsArray[i]); 
        }
        free(pointsArray);
        return NULL;
    }
    return pointsArray;
}

double spRGBHistL2Distance(SPPoint** rgbHistA, SPPoint** rgbHistB) {
	double averageDistance = 0;

	//Go over each channel and calculate the L2 distance between hist vectors in A and B
	//Add all distances multiplied by 0.33 to get the average distance
	for(int i=0; i < NUM_OF_CHANNELS; ++i)
	{
		if (rgbHistA[i] == NULL || rgbHistB[i] == NULL)
			return ERROR_CODE;//A null pointer in one of the channels. Distance can't be calculated. */

		averageDistance += CHANNEL_L2_DISTANCE_PROPORTION * spPointL2SquaredDistance(rgbHistA[i], rgbHistB[i]);
	}

	return averageDistance;
}

SPPoint** spGetSiftDescriptors(const char* str, int imageIndex, int nFeaturesToExtract, int *nFeatures) {
    if (str == NULL || nFeaturesToExtract <= 0 || nFeatures == NULL) {
        return NULL;
    }
    SPPoint ** pointsArray = NULL;
    cv::Mat src;

    /* Load img - gray scale mode! */
    src = cv::imread(str, CV_LOAD_IMAGE_GRAYSCALE);
    if (src.empty()) {
        printf(EMPTY_IMAGE_LOADED_ERROR_FORMAT,EMPTY_IMAGE_LOADED_ERROR, str);
        exit(ERROR_CODE);

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
    *nFeatures = ds1.rows;

    /* allocate nFeatures points */
    pointsArray = (SPPoint**)malloc(sizeof(*pointsArray) * *nFeatures);
    if (pointsArray == NULL) {
        return NULL;
    }

    /* create an array of nFeatures points, each containing the corresponding descriptor */
    bool pointsCreateFailure = false;
    double * featuresData = NULL;
    featuresData = (double*)malloc(sizeof(*featuresData) * ds1.cols);
    if (featuresData == NULL) {
       pointsCreateFailure = true;
    }
    for (int i = 0; i < *nFeatures && !pointsCreateFailure; ++i) {
        for (int j = 0; j < ds1.cols; ++j) {
            featuresData[j] = ds1.at<float>(i,j);
        }
        pointsArray[i] = spPointCreate(featuresData, ds1.cols, imageIndex);
        if (pointsArray[i] == NULL) {
            pointsCreateFailure = true;
        }
    }

    free(featuresData);
    if (pointsCreateFailure) {
        for (int i = 0; i < *nFeatures; ++i) {
            spPointDestroy(pointsArray[i]); 
        }
        free(pointsArray);
        pointsArray = NULL;
    }

    return pointsArray;
}

int* spBestSIFTL2SquaredDistance(int kClosest, SPPoint* queryFeature, SPPoint*** databaseFeatures, int numberOfImages, int* nFeaturesPerImage)
{
	/*Input validation*/
	if (queryFeature == NULL ||
		databaseFeatures == NULL ||
		numberOfImages <= 1 ||
		nFeaturesPerImage == NULL)
		return NULL;

	/*Whether the procedure should keep running or encountered a memory allocation error and should exit*/
	bool isKeepRunning = true;

	/*Allocate memory for storing the kClosest indices of images closest to queryFeature*/
	int* closestImgIndices = (int*)malloc(kClosest*sizeof(*closestImgIndices));

	/*Create a priority queue of size kClosest*/
	SPBPQueue* priorityQueue = spBPQueueCreate(kClosest);

	if (closestImgIndices == NULL || priorityQueue == NULL)
		isKeepRunning = false; /*Failed to allocate memory*/

	if (isKeepRunning)
	{
		for(int i = 0; i < numberOfImages; ++i) /*Go over each image*/
			for(int j = 0; j < nFeaturesPerImage[i]; ++j) /*Go over each feature in image*/
			{
				/*Calculate the L2 distance between the feature and queryFeature*/
				double distance = spPointL2SquaredDistance(databaseFeatures[i][j], queryFeature);

				/*Enqueue the L2 distance to the priority queue, with the image's index*/
				SP_BPQUEUE_MSG msg = spBPQueueEnqueue(priorityQueue, i, distance);

				if (msg == SP_BPQUEUE_OUT_OF_MEMORY)
				{
					isKeepRunning = false;
					break;
				}
			}
	}

	if (isKeepRunning)
	{
		/*Get the size of the queue, in case it's smaller than kClosest*/
		int queueSize = spBPQueueSize(priorityQueue);
		BPQueueElement queueElem; /*A helper to retrieve elements from the priority queue*/

		/*Go over all elements in the priority queue to retrieve image indices*/
		for(int i = 0; i < queueSize; ++i)
		{
			/*Get the element with the lowest value in the queue*/
			spBPQueuePeek(priorityQueue, &queueElem);

			/*Add the index of the queue element to the output array*/
			closestImgIndices[i] = queueElem.index;

			/*Dequeue the lowest value element in preparation for the next iteration*/
			spBPQueueDequeue(priorityQueue);
		}
	}


	/*Free all allocated memory for the queue*/
	spBPQueueDestroy(priorityQueue);

	return closestImgIndices;
}
