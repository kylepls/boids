
#ifndef TEST_SETTINGS_H
#define TEST_SETTINGS_H

#include <stdbool.h>
#include <math/vec3.h>

typedef struct {
    float alignment_radius;
    float alignment_multiplier;

    float cohesion_radius;
    float cohesion_multiplier;

    float separation_radius;
    float separation_multiplier;

    float flock_radius;
    int flock_size;
    float flock_multiplier;

    float max_velocity;
    float target_velocity;
} BoidsSettings;

typedef struct {
    bool light_demo;
    bool normals;
    bool fog;
    bool fish;
    bool caustic_demo;
    bool fps;
    bool occlusion;
    bool animate;
    bool fish_chunks;
    bool sample_fish;
    bool birds_eye_view;
    bool points_shader;
    bool points_shader_iso;
    bool world;
    bool caustic;
    bool debug_text;
} AdvancedSettings;

typedef struct {
    // view settings
    int field_of_view;
    float aspect_ratio;
    float dim;

    // game settings
    bool debug;
    bool pause;
    bool adaptive_draw_settings;

    // world settings
    float iso_level;
    float persistence;
    float lacunarity;
    float weight_multiplier;
    float noise_offset;
    float noise_scale;
    float world_scale;
    int octaves;
    float *octave_offsets;

    float render_distance;
    float render_height;
    float fog_density;
    vec3 fog_color;
    float fog_linear_max;
    float fog_linear_min;
    float caustic_intensity;
    float caustic_scale;

    BoidsSettings boids;

    AdvancedSettings adv;

    bool drawWorld;
} GameSettings;

extern GameSettings settings;

typedef struct {
    float target_fps;
} AdaptiveSettings;

extern AdaptiveSettings adaptive_settings;

#endif //TEST_SETTINGS_H
