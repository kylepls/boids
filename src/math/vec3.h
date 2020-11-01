
#ifndef TEST_VEC3_H
#define TEST_VEC3_H

typedef struct {
    float x;
    float y;
    float z;
} vec3;

float dotv3(vec3 v1, vec3 v2);

vec3 normv3(vec3 vector);

vec3 crossv3(const vec3 v1, const vec3 v2);

float lenv3(const vec3 vector);

vec3 lerpv3(const vec3 start, const vec3 end, float t);

vec3 subtractv3(const vec3 x, const vec3 y);

vec3 multv3(const vec3 v1, const vec3 v2);

vec3 negv3(const vec3 vector);

vec3 plusv3(const vec3 x, const vec3 y);

vec3 clampv3(vec3 target, float max);

vec3 multv3f(const vec3 x, const float y);

void printv3(vec3 print);


#endif //TEST_VEC3_H
