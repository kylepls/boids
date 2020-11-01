#include "stdlib.h"
#include "boids_chunks.h"
#include "../math/extramath.h"
#include "stdio.h"
#include "../window.h"
#include "../shaders/shaders.h"
#include "../math/view_frustum.h"
#include "boids.h"
#include "boids_collision.h"
#include "../settings.h"
#include "../player/player.h"

#define BOIDS_CHUNK_BUFFER_WIDTH BOIDS_BUFFER_SIZE/9
#define BOIDS_PER_CHUNK BOIDS_CHUNK_BUFFER_WIDTH/3
static Boid boids[BOIDS_BUFFER_SIZE];
static int boids_c = 0;

#define CHUNK_BUFFER_WIDTH 3
#define CHUNK_SIZE 16
#define MAX_BUFFER_INDEX (CHUNK_BUFFER_WIDTH*CHUNK_BUFFER_WIDTH)
static Chunk chunks[MAX_BUFFER_INDEX];

int get_chunk_index(vec3 worldPoint) {
    int player_cx = (int) floorf(player.position.x / CHUNK_SIZE);
    int player_cz = (int) floorf(player.position.z / CHUNK_SIZE);

    int point_cx = (int) floorf(worldPoint.x / CHUNK_SIZE);
    int point_cz = (int) floorf(worldPoint.z / CHUNK_SIZE);

    int dx = point_cx - player_cx;
    int dz = point_cz - player_cz;

    if (abs(dx) > CHUNK_BUFFER_WIDTH / 2 || abs(dz) > CHUNK_BUFFER_WIDTH / 2) {
        return -1;
    } else {
        int middleChunk = (CHUNK_BUFFER_WIDTH * CHUNK_BUFFER_WIDTH) / 2;
        return middleChunk + dz * CHUNK_BUFFER_WIDTH + dx;
    }
}

Boid *generate_boid(int bufferIndex) {
    int chunk_dx = bufferIndex % CHUNK_BUFFER_WIDTH - (CHUNK_BUFFER_WIDTH / 2);
    int chunk_dz = bufferIndex / CHUNK_BUFFER_WIDTH - (CHUNK_BUFFER_WIDTH / 2);

    int player_chunk_x = (int) floorf(player.position.x / 16);
    int player_chunk_z = (int) floorf(player.position.z / 16);

    int chunk_x = player_chunk_x + chunk_dx;
    int chunk_z = player_chunk_z + chunk_dz;

    vec3 chunkMin = {chunk_x * 16, 0, chunk_z * 16};
    vec3 chunkMax = {(chunk_x + 1) * 16, 0, (chunk_z + 1) * 16};

    // find where to spawn it
    float px;
    float py = 15;
    float pz;

    if (bufferIndex < CHUNK_BUFFER_WIDTH) {
        // spawn on bottom
        pz = chunkMin.z;
        px = chunkMin.x + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
    } else if (bufferIndex % CHUNK_BUFFER_WIDTH == 0) {
        // spawn on right
        px = chunkMin.x;
        pz = chunkMin.z + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
    } else if (bufferIndex % CHUNK_BUFFER_WIDTH == CHUNK_BUFFER_WIDTH - 1) {
        // spawn on left
        px = chunkMax.x;
        pz = chunkMin.z + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
    } else if (bufferIndex / CHUNK_BUFFER_WIDTH == CHUNK_BUFFER_WIDTH - 1) {
        // spawn on top
        pz = chunkMax.z;
        px = chunkMin.x + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
    } else {
        // spawn anywhere
        px = chunkMin.x + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
        pz = chunkMin.z + ((rand() / (float) RAND_MAX) * CHUNK_SIZE);
    }

    vec3 position = {px, py, pz};
    vec3 velocity = {
            settings.boids.max_velocity * randSign(),
            settings.boids.max_velocity * randSign(),
            settings.boids.max_velocity * randSign(),
    };
    Boid boid = {position, velocity};
    if (boids_c > BOIDS_BUFFER_SIZE) {
        printf("Too many boids\n");
        fflush(stdout);
        exit(0);
    }
    boids[boids_c++] = boid;
    return &boids[boids_c - 1];
}

void fill_chunk_buffer() {
    for (int i = 0; i < MAX_BUFFER_INDEX; i++) {
        Chunk *chunk = &chunks[i];
        chunk->boids_c = 0;
    }

    for (int i = 0; i < boids_c; i++) {
        Boid *boid = &boids[i];
        int index = get_chunk_index(boid->position);
        if (index == -1) {
            // outside of bounds
            for (int j = i; j < boids_c - 1; j++) {
                boids[j] = boids[j + 1];
            }

            i--;
            boids_c--;
        } else {
            Chunk *chunk = &chunks[index];
            chunk->boids[chunk->boids_c++] = boid;
        }
    }
}

void generate_chunks() {
    for (int i = 0; i < MAX_BUFFER_INDEX; i++) {
        Boid **chunkBoids = (Boid **) malloc(BOIDS_CHUNK_BUFFER_WIDTH * sizeof(Boid *));
        chunks[i] = (Chunk) {chunkBoids, 0};
    }
}

void tick_chunks(float dt) {
    if (!settings.pause) {
        fill_chunk_buffer();
        for (int i = 0; i < MAX_BUFFER_INDEX; i++) {
            Chunk *chunk = &chunks[i];
            while (chunk->boids_c < BOIDS_PER_CHUNK) {
                Boid *boid = generate_boid(i);
                chunk->boids[chunk->boids_c++] = boid;
            }

            update_velocity(chunk->boids, chunk->boids_c, dt);
        }
        apply_boid_collision(boids, boids_c, dt);
    }
}

void move_boids(float dt) {
    if (!settings.pause) {
        for (int i = 0; i < boids_c; i++) {
            Boid *boid = &boids[i];
            boid->position = plusv3(boid->position, multv3f(boid->velocity, dt));
        }
    }
}

void draw_chunked_boids(float timeMs) {
    for (int i = 0; i < MAX_BUFFER_INDEX; i++) {
        Chunk *chunk = &chunks[i];

        int dx = i % CHUNK_BUFFER_WIDTH - CHUNK_BUFFER_WIDTH / 2;
        int dz = i / CHUNK_BUFFER_WIDTH - CHUNK_BUFFER_WIDTH / 2;

        int player_cx = (int) floorf(player.position.x / CHUNK_SIZE);
        int player_cz = (int) floorf(player.position.z / CHUNK_SIZE);

        float world_x = (player_cx + dx) * 16;
        float world_z = (player_cz + dz) * 16;

        world_x += CHUNK_SIZE / 2;
        world_z += CHUNK_SIZE / 2;

        if (settings.debug && settings.adv.fish_chunks) {
            glRasterPos3d(world_x, 20, world_z);
            glColor3f(1, 1, 1);
            Print("Chunk: %d : %d", i, chunk->boids_c);

            glColor3f(0, 1, 0);
            glBegin(GL_LINES);
            for (int b = 0; b < chunk->boids_c; b++) {
                glVertex3f(world_x, 20, world_z);
                vec3 pos = chunk->boids[b]->position;
                glVertex3f(pos.x, pos.y, pos.z);
            }
            glEnd();
        }
    }

    if (settings.adv.sample_fish) {
        Boid boid = (Boid) {(vec3) {38, 6.9, 20.9}, (vec3) {0, 0, 1}, (vec3) {0, 0, 0}};
        drawBoid(&boid, timeMs);
        return;
    }

    for (int i = 0; i < boids_c; i++) {
        Boid *boid = &boids[i];
        if (pointInFrustum(boid->position.x, boid->position.y, boid->position.z)) {
            drawBoid(boid, timeMs);
        }
    }
}

