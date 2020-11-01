#include <caustic/caustic.h>
#include "time.h"
#include "lib/CSCIx229.h"
#include "window.h"
#include "boids/boids.h"
#include "math.h"
#include "math/extramath.h"
#include "world/marching.h"
#include "boids/boids_chunks.h"
#include "world/world.h"
#include "settings.h"
#include "player/player.h"
#include "world/adaptive_draw.h"
#include "textbox.h"

float timeDelta = 0;
float lastUpdate = 0;
float fps = 0;
float fpsClock = 0;
int frames = 0;
long lastTick = -50;

bool first_frame = true;

void idle() {
    glutPostRedisplay();
}

void display() {
    if (first_frame) {
        generate_chunks();
        init_world();
        first_frame = false;
    }
    frames++;

    if (!settings.debug) {
        glClearColor(settings.fog_color.x, settings.fog_color.y, settings.fog_color.z, 1);
    } else {
        glClearColor(0, 0, 0, 1);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();

    double Ex = player.position.x;
    double Ey = player.position.y;
    double Ez = player.position.z;

    float th = player.th;
    float ph = player.ph;
    double Cx = 2 * settings.dim * Sin(th);
    double Cy = 2 * settings.dim * Tan(ph);
    double Cz = -2 * settings.dim * Cos(th);

    double upX = 0;
    double upY = Cos(ph);
    double upZ = 0;

    if (settings.adv.birds_eye_view) {
        gluLookAt(Ex, 40, Ez, Ex + Cx, Ey + Cy, Cz + Ez, upX, upY, upZ);
    } else {
        gluLookAt(Ex, Ey, Ez, Ex + Cx, Ey + Cy, Cz + Ez, upX, upY, upZ);
    }

    float currentTimeMs = glutGet(GLUT_ELAPSED_TIME);
    float currentTimeSeconds = currentTimeMs / 1000.0;
    timeDelta = currentTimeSeconds - lastUpdate;
    lastUpdate = currentTimeSeconds;
    if (currentTimeSeconds - fpsClock > 1) {
        fps = frames / (currentTimeSeconds - fpsClock);
        fpsClock = currentTimeSeconds;
        frames = 0;

        if (settings.adaptive_draw_settings) {
            update_world_settings();
        }
    }

    tick_chunks((currentTimeMs - lastTick) / 1000.0);
    lastTick = currentTimeMs;

    if (settings.drawWorld && settings.adv.world) { draw_world(currentTimeMs); }
    if (settings.adv.fish) {
        move_boids(timeDelta);
        draw_chunked_boids(currentTimeMs);
    }
    move_player(timeDelta);

    if (settings.debug && settings.adv.debug_text) {
        glColor3f(1, 1, 1);

        glWindowPos2i(5, 165);
        Print("FPS: %.1f", fps);

        glWindowPos2i(5, 145);
        Print(
                "x:%.1f y:%.1f z:%.1f th:%.1f ph:%.1f",
                player.position.x,
                player.position.y,
                player.position.z,
                player.th,
                player.ph
        );

        // World
        glWindowPos2i(5, 125);
        Print("World - Iso Level (c): %.1f, Octaves (v): %d, Persistence (b): %.2f, Lacunarity (n): %.2f",
              settings.iso_level,
              settings.octaves,
              settings.persistence,
              settings.lacunarity);

        // Boids
        glWindowPos2i(5, 105);
        Print("Cohesion Radius (1): %.2f Multiplier (2): %.2f", settings.boids.cohesion_radius,
              settings.boids.cohesion_multiplier);
        glWindowPos2i(5, 85);
        Print("Separation Radius (3): %.2f Multiplier (4): %.2f", settings.boids.separation_radius,
              settings.boids.separation_multiplier);
        glWindowPos2i(5, 65);
        Print("Alignment Radius (5): %.2f Multiplier (6): %.2f", settings.boids.alignment_radius,
              settings.boids.alignment_multiplier);
        glWindowPos2i(5, 45);
        Print("Max velocity (7): %.2f", settings.boids.max_velocity);

        glWindowPos2i(5, 25);
        Print("Fog Multiplier (8): %.2f, Caustic Intensity (9): %.2f, Caustic Scale (0): %.2f", settings.fog_density,
              settings.caustic_intensity, settings.caustic_scale);
    }

    if (settings.adv.fps) {
        glColor3f(1, 1, 1);
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        glWindowPos2i(w - 110, h - 20);
        Print("FPS: %.2f", fps);
    }

    draw_textbox();

    glutSwapBuffers();
    ErrCheck("display");
    glFlush();
}

int main(int argc, char *argv[]) {
    settings.iso_level = 0.45;
    settings.persistence = 0.54;
    settings.lacunarity = 2.0;
    settings.weight_multiplier = 11.24;
    settings.noise_offset = 1032489.128;
    settings.noise_scale = 1.71;
    settings.octaves = 8;
    settings.world_scale = 1;

    float octave_offsets[] = {129, 238, 200, 2398, 28923, 2389, 383, 20};
    settings.octave_offsets = (float *) malloc(settings.octaves * sizeof(float));
    memcpy(settings.octave_offsets, octave_offsets, settings.octaves * sizeof(float));

    settings.render_distance = 30;
    settings.render_height = 18;
    settings.fog_density = 0.03;
    settings.fog_color = (vec3) {0, 0, 1};
    settings.fog_linear_max = 40;
    settings.fog_linear_min = 30;

    settings.caustic_intensity = 0.2;
    settings.caustic_scale = 0.5;

    settings.field_of_view = 55;
    settings.aspect_ratio = 1;
    settings.dim = 40;

    settings.debug = false;
    settings.pause = false;
    settings.adaptive_draw_settings = false;

    settings.boids.separation_radius = 0.5;
    settings.boids.separation_multiplier = 1.3;

    settings.boids.cohesion_radius = 2.0;
    settings.boids.cohesion_multiplier = 0.5;

    settings.boids.alignment_radius = 1.25;
    settings.boids.alignment_multiplier = 0.2;

    settings.boids.max_velocity = 3.0;
    settings.boids.target_velocity = 2.0;

    settings.boids.flock_radius = settings.boids.cohesion_radius * 1.5;
    settings.boids.flock_size = 8;
    settings.boids.flock_multiplier = 0.3;

    settings.drawWorld = true;
    settings.adv.world = true;

    player.position = (vec3) {36.6, 6.8, 19.9};
    player.th = 125;
    player.ph = -5.1;

    adaptive_settings.target_fps = 60;

    settings.adv.light_demo = false;
    settings.adv.normals = false;
    settings.adv.fog = true;
    settings.adv.fish = true;
    settings.adv.caustic_demo = false;
    settings.adv.fps = false;
    settings.adv.occlusion = true;
    settings.adv.animate = true;
    settings.adv.fish_chunks = true;
    settings.adv.sample_fish = false;
    settings.adv.birds_eye_view = false;
    settings.adv.points_shader = false;
    settings.adv.caustic = true;
    settings.adv.debug_text = true;

    srand(time(NULL));

    glutInit(&argc, argv);
    setup_window(display);
}
