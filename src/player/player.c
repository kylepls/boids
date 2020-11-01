#include "player.h"
#include "../lib/CSCIx229.h"
#include "../math/extramath.h"

Player player;

void move_player(float dt) {
    float maxSpeed = 4.0F; // 0.5 units/second
    float accelerationSpeed = maxSpeed;

    float decayFactor = -accelerationSpeed / 2;
    vec3 decay = multv3f(player.velocity, decayFactor);
    player.velocity = plusv3(player.velocity, multv3f(decay, dt));

    vec3 result = {
            Sin(player.th) * player.acceleration.x - Cos(player.th) * player.acceleration.z,
            Tan(player.ph) * player.acceleration.x,
            -Cos(player.th) * player.acceleration.x - Sin(player.th) * player.acceleration.z,
    };
    vec3 accelerationDelta = multv3f(result, accelerationSpeed * dt);

    player.velocity = plusv3(player.velocity, accelerationDelta);
    player.velocity = clampv3(player.velocity, maxSpeed);

    if (lenv3(player.velocity) < maxSpeed / 10000) {
        player.velocity.x = 0;
        player.velocity.y = 0;
        player.velocity.z = 0;
        return;
    }

    player.position = plusv3(player.position, multv3f(player.velocity, dt));
}
