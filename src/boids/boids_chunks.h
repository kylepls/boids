#ifndef TEST_BOIDS_CHUNKS_H
#define TEST_BOIDS_CHUNKS_H

#include "boids.h"

#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 20

#define BOIDS_BUFFER_SIZE 900

typedef struct {
    Boid **boids;
    int boids_c;
} Chunk;

void generate_chunks();

void tick_chunks(float dt);

void move_boids(float dt);

void draw_chunked_boids(float timeMs);

#endif //TEST_BOIDS_CHUNKS_H
