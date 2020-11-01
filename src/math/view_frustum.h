#ifndef TEST_VIEW_FRUSTUM_H
#define TEST_VIEW_FRUSTUM_H

#include <stdbool.h>

void extractFrustum();

bool pointInFrustum(float x, float y, float z);

#endif //TEST_VIEW_FRUSTUM_H
