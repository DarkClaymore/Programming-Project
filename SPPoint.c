#include "SPPoint.h"
#include <stdlib.h>
#include <assert.h>
struct sp_point_t{
	int dim; /*the dimension of the point*/
	int index; /* the index of the point*/
	double* coordinates; /*All the coordinates of the point*/

};


SPPoint* spPointCreate(double* data, int dim, int index){

	if (dim <= 0) return NULL; /*Invalid dimension*/
	if (index < 0) return NULL; /*Invalid index*/

	 /*Allocate memory for the point*/
	SPPoint* newPoint = (SPPoint*)malloc(sizeof(SPPoint));
	if (newPoint == NULL)
		return NULL; /*Failed to allocate memory*/


	/*Allocate memory for the coordinates*/
	double* coordinates =  (double*)malloc(dim*sizeof(double));
	if (coordinates == NULL) return NULL; /*Failed to allocate memory*/

	/*Load all coordinates into the point*/
	for (int i = 0; i<dim; ++i) {
		coordinates[i]=data[i];
	}
	newPoint->coordinates = coordinates;
	newPoint->dim=dim;
	newPoint->index=index;
	return newPoint;
}

SPPoint* spPointCopy(SPPoint* source)
{
	assert(source != NULL); /*Make sure that the source isn't null*/

	/*Return a new point with source's data*/
	return spPointCreate(source->coordinates,source->dim,source->index);
}

void spPointDestroy(SPPoint* point){

	if (point == NULL) return; /*Not pointing to any object*/

	/*First free the memory for the coordinates*/
	free(point->coordinates);
	/*Then free memory for the whole object*/
	free(point);
}

int spPointGetDimension(SPPoint* point){
	assert(point != NULL); /*Make sure that the point isn't null*/
	return point->dim;
}

int spPointGetIndex(SPPoint* point){
	assert(point != NULL); /*Make sure that the point isn't null*/
	return point->index;
}

double spPointGetAxisCoor(SPPoint* point, int axis){
	assert(point != NULL); /*Make sure that the point isn't null*/
	assert(axis <= point->dim); /*Make sure the axis isn't bigger than the point's dimension*/
	return point->coordinates[axis]; /*Return the requested coordinate*/
}
double spPointL2SquaredDistance(SPPoint* p, SPPoint* q){
	assert(p != NULL && q != NULL); /*Make sure that the points aren't null*/
	assert(p->dim == q->dim); /*Make sure both points have the same dimension*/

	double res = 0.0;
	for (int i = 0; i < q->dim; ++i) /*L2 calculation*/
	{
		/*Add new element to the L2 result: (p[i] - q[i])^2*/
		res+=(p->coordinates[i]-q->coordinates[i])*(p->coordinates[i]-q->coordinates[i]);
	}
	return res;
}

