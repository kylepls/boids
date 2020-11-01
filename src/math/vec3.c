#include "stdio.h"
#include "math.h"
#include "vec3.h"

float dotv3(vec3 v1, vec3 v2) {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

float lenv3(const vec3 vector) {
    return sqrtf((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

vec3 lerpv3(const vec3 start, const vec3 end, float t) {
    t = fmin(t, 1);
    t = fmax(t, 0);
    return (vec3) {
            start.x + t * (end.x - start.x),
            start.y + t * (end.y - start.y),
            start.z + t * (end.z - start.z)
    };
}

vec3 negv3(const vec3 vector) {
    return (vec3) {-vector.x, -vector.y, -vector.z};
}

vec3 clampv3(vec3 target, float max) {
    if (lenv3(target) > max) {
        return multv3f(normv3(target), max);
    } else {
        return target;
    }
}

vec3 plusv3(const vec3 x, const vec3 y) {
    return (vec3) {
            x.x + y.x,
            x.y + y.y,
            x.z + y.z,
    };
}

vec3 subtractv3(const vec3 x, const vec3 y) {
    return (vec3) {
            x.x - y.x,
            x.y - y.y,
            x.z - y.z,
    };
}

vec3 multv3(const vec3 v1, const vec3 v2) {
    return (vec3) {
            v1.x * v2.x,
            v1.y * v2.y,
            v1.z * v2.z,
    };
}

vec3 multv3f(const vec3 x, const float y) {
    vec3 result = {
            x.x * y,
            x.y * y,
            x.z * y,
    };
    return result;
}

vec3 normv3(vec3 vector) {
    float length = lenv3(vector);
    if (length != 0) {
        return (vec3) {
                vector.x / length,
                vector.y / length,
                vector.z / length
        };
    } else {
        return (vec3) {0, 0, 0};
    }
}

vec3 crossv3(const vec3 v1, const vec3 v2) {
    return (vec3) {v1.y * v2.z - v1.z * v2.y,
                   v1.z * v2.x - v1.x * v2.z,
                   v1.x * v2.y - v1.y * v2.x};
}

void printv3(vec3 print) {
    printf("[x:%.2f, y:%.2f, z:%.2f]", print.x, print.y, print.z);
}

