#ifndef TEST_EXTRAMATH_H
#define TEST_EXTRAMATH_H

#include "stdbool.h"
#include "vec3.h"

#define PI 3.1415926
#define Tan(th) tanf(PI/180*(th))

void lookAt(vec3 position, vec3 target);

vec3 relativeTo(vec3 P, vec3 vector, vec3 offset);

float randSign();

#endif //TEST_EXTRAMATH_H
