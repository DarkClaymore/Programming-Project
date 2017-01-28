#include "SPPoint.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


struct sp_point_t {
    double* data;
    int dim;
    int index;
};


SPPoint* spPointCreate(double* data, int dim, int index) {

    if (data == NULL || dim <= 0 || index < 0) {
        return NULL;
    }

    int sizeOfCoords = sizeof(*data) * dim;

    SPPoint * newPoint = malloc(sizeof(*newPoint));

    if (newPoint == NULL) {
        return NULL;
    }

    double * newData = malloc(sizeOfCoords);
    if (newData == NULL) {
        spPointDestroy(newPoint);
        return NULL;
    }

    newPoint-> index = index;
    newPoint-> dim = dim;
    newPoint->data = newData;
    memcpy(newData, data, sizeOfCoords);
    return newPoint;
}

SPPoint* spPointCopy(SPPoint* source) {
    assert(source != NULL);

    int sizeOfCoords = sizeof(*source->data) * source->dim;

    SPPoint * newPoint = malloc(sizeof(*newPoint));
    if (newPoint == NULL) {
        return NULL;
    }

    double * newData = malloc(sizeOfCoords);
    if (newData == NULL) {
        spPointDestroy(newPoint);
        return NULL;
    }
    

    newPoint->dim = source->dim;
    newPoint->index = source->index;
    newPoint->data = newData;
    memcpy(newData, source->data, sizeOfCoords);

    return newPoint;

}

void spPointDestroy(SPPoint* point) {
    if (point != NULL) {
       free(point->data);
       free(point);
    }

}

int spPointGetDimension(SPPoint* point) {
    assert(point != NULL);
    return point->dim;
}

int spPointGetIndex(SPPoint* point) {
    assert(point != NULL);
    return point->index;
}


double spPointGetAxisCoor(SPPoint* point, int axis) {
    assert(point != NULL && axis < point->dim);
    return point->data[axis];
}

double spPointL2SquaredDistance(SPPoint* p, SPPoint* q) {
    assert(p != NULL && q != NULL && p->dim == q->dim);
    double * pData = p->data;
    double * qData = q->data;
    double distance = 0;
    for (int i = 0; i < p->dim; i++) {
       distance += (pData[i] - qData[i])*(pData[i] - qData[i]);
    }
    return distance;

}
