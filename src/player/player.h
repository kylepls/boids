
#ifndef TEST_PLAYER_H
#define TEST_PLAYER_H

#include "../math/vec3.h"

typedef struct {
    vec3 acceleration;
    vec3 velocity;
    vec3 position;
    float th;
    float ph;
} Player;

extern Player player;

void move_player(float dt);

#endif //TEST_PLAYER_H
