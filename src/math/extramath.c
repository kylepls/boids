#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "extramath.h"
#include "../lib/CSCIx229.h"
#include "math.h"

void lookAt(vec3 position, vec3 target) {
    vec3 T = normv3(target);
    vec3 Y = {0, 1, 0};
    vec3 L = normv3(crossv3(T, Y));
    vec3 U = normv3(crossv3(L, T));

    double glMatrix[] = {
            L.x, L.y, L.z, 0,
            T.x, T.y, T.z, 0,
            U.x, U.y, U.z, 0,
            position.x, position.y, position.z, 1
    };

    glMultMatrixd(glMatrix);
}

vec3 relativeTo(vec3 P, vec3 vector, vec3 offset) {
    vec3 T = normv3(vector);
    vec3 Y = {0, 1, 0};
    vec3 L = normv3(crossv3(T, Y));
    vec3 U = normv3(crossv3(L, T));

    vec3 pos = {
            (L.x * offset.x) + (T.x * offset.y) + (U.x * offset.z) + P.x,
            (L.y * offset.x) + (T.y * offset.y) + (U.y * offset.z) + P.y,
            (L.z * offset.x) + (T.z * offset.y) + (U.z * offset.z) + P.z,
    };
    return pos;
}

float randSign() {
    return rand() % 2 == 1 ? 1.0F : -1.0F;
}

