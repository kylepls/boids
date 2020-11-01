#ifndef TEST_BOIDS_H
#define TEST_BOIDS_H

#include "../math/extramath.h"

typedef struct {
    vec3 position;
    vec3 velocity;
    vec3 new_velocity;
} Boid;

void update_velocity(Boid **boids, int boids_c, float dt);

void clamp_velocity(Boid **boids, int boids_c, float dt);

void drawBoid(Boid *boid, float timeMs);

#endif //TEST_BOIDS_H
