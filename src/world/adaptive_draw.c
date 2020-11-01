#include <math.h>
#include <stdio.h>
#include "adaptive_draw.h"
#include "../main.h"
#include "../settings.h"

void update_world_settings() {
    if (fps < adaptive_settings.target_fps) {
        settings.render_distance -= (int) fmin((adaptive_settings.target_fps - fps) / 4, 5);
        printf("Decreasing render distance, new = %.1f\n", settings.render_distance);
        fflush(stdout);
    }
}