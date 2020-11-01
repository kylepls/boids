#include <stdlib.h>
#include <unistd.h>
#include <player/player.h>
#include "boids.h"
#include "stdio.h"
#include "../math/extramath.h"
#include "../window.h"
#include "../world/marching.h"
#include "../lib/CSCIx229.h"
#include "boids_chunks.h"
#include "../shaders/shaders.h"
#include "boids_collision.h"
#include "../settings.h"

#define VERT_FILE "src/shaders/fish.vert"
#define FRAG_FILE "src/shaders/fish.frag"

#define GEOM_FILE_DEBUG "src/shaders/fish_debug.geom"
#define VERT_FILE_DEBUG "src/shaders/fish_debug.vert"
#define FRAG_FILE_DEBUG "src/shaders/fish_debug.frag"

static int obj = -1;

void apply_cohesion(Boid **boids, int boids_c, float dt) {
    // find boids in a radius
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        vec3 nearby = {0, 0, 0};
        int nearby_count = 0;
        for (int j = 0; j < boids_c; j++) {
            if (j == i) { continue; }
            Boid *target = boids[j];
            vec3 delta = subtractv3(target->position, boid->position);
            if (lenv3(delta) < settings.boids.cohesion_radius) {
                nearby_count += 1;
                nearby = plusv3(nearby, delta);
            }
        }

        if (nearby_count > 0) {
            vec3 target_velocity = multv3f(normv3(nearby), settings.boids.max_velocity);
            boid->new_velocity = lerpv3(boid->new_velocity, target_velocity, dt * settings.boids.cohesion_multiplier);
        }
    }
}

void apply_alignment(Boid **boids, int boids_c, float dt) {
    // find boids in a radius
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        vec3 nearby = {0, 0, 0};
        int nearby_count = 0;
        for (int j = 0; j < boids_c; j++) {
            if (j == i) { continue; }
            Boid *target = boids[j];
            vec3 delta = subtractv3(target->position, boid->position);
            if (lenv3(delta) < settings.boids.alignment_radius) {
                nearby_count += 1;
                nearby = plusv3(nearby, target->velocity);
            }
        }

        if (nearby_count > 0) {
            vec3 target_velocity = multv3f(normv3(nearby), settings.boids.max_velocity);
            boid->new_velocity = lerpv3(boid->new_velocity, target_velocity, dt * settings.boids.alignment_multiplier);
        }
    }
}

// limit the size of a flock
// this negates cohesion if a flock gets too big
void apply_flock_size(Boid **boids, int boids_c, float dt) {
    // find boids in a radius
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        vec3 nearby = {0, 0, 0};
        int nearby_count = 0;
        for (int j = 0; j < boids_c; j++) {
            if (j == i) { continue; }
            Boid *target = boids[j];
            vec3 delta = subtractv3(target->position, boid->position);
            if (lenv3(delta) < settings.boids.flock_radius) {
                nearby_count += 1;
                nearby = plusv3(nearby, delta);
            }
        }

        if (nearby_count >= settings.boids.flock_size) {
            vec3 target_velocity = multv3f(negv3(normv3(nearby)), settings.boids.max_velocity);
            boid->new_velocity = lerpv3(boid->new_velocity, target_velocity, dt * settings.boids.flock_multiplier);
        }
    }
}

void apply_separation(Boid **boids, int boids_c, float dt) {
    // find boids in a radius
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        vec3 nearby = {0, 0, 0};
        int nearby_count = 0;
        for (int j = 0; j < boids_c; j++) {
            if (j == i) { continue; }
            Boid *target = boids[j];
            vec3 delta = subtractv3(target->position, boid->position);
            if (lenv3(delta) < settings.boids.separation_radius) {
                nearby_count += 1;
                nearby = plusv3(nearby, multv3f(delta, 1 / (lenv3(delta) + 0.001)));
            }
        }

        if (nearby_count > 0) {
            vec3 target_velocity = negv3(multv3f(normv3(nearby), settings.boids.max_velocity));
            boid->new_velocity = lerpv3(boid->new_velocity, target_velocity, dt * settings.boids.separation_multiplier);
        }
    }
}

void clamp_velocity(Boid **boids, int boids_c, float dt) {
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        if (lenv3(boid->velocity) > settings.boids.max_velocity) {
            boid->velocity = multv3f(normv3(boid->velocity), settings.boids.max_velocity);
        } else if (lenv3(boid->velocity) < settings.boids.target_velocity) {
            boid->velocity = lerpv3(boid->velocity, multv3f(normv3(boid->velocity), settings.boids.target_velocity),
                                    dt);
        }
    }
}

void apply_bounds(Boid **boids, int boids_c, float dt) {
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = boids[i];
        vec3 point = boid->position;
        if (point.y < 0 || point.y >= CHUNK_HEIGHT) {
            vec3 correction;
            if (point.y < 0) {
                correction = (vec3) {0, 1, 0};
            } else {
                correction = (vec3) {0, -1, 0};
            }
            correction = multv3f(normv3(correction), settings.boids.max_velocity);

            boid->new_velocity = lerpv3(boid->new_velocity, correction, dt);
        }
    }
}

void update_velocity(Boid **boids, int boids_c, float dt) {
    for (int i = 0; i < boids_c; i++) {
        boids[i]->new_velocity = boids[i]->velocity;
    }
    apply_cohesion(boids, boids_c, dt);
    apply_separation(boids, boids_c, dt);
    apply_alignment(boids, boids_c, dt);
    apply_flock_size(boids, boids_c, dt);

    apply_bounds(boids, boids_c, dt);
    for (int i = 0; i < boids_c; i++) {
        boids[i]->velocity = boids[i]->new_velocity;
    }

    clamp_velocity(boids, boids_c, dt);
}

static int init_program() {
    int prog = glCreateProgram();

    int vert = CreateShader(GL_VERTEX_SHADER, VERT_FILE);
    glAttachShader(prog, vert);
    int frag = CreateShader(GL_FRAGMENT_SHADER, FRAG_FILE);
    glAttachShader(prog, frag);

    glLinkProgram(prog);
    PrintProgramLog(prog);
    return prog;
}

static int init_program_debug() {
    int prog = glCreateProgram();

    int vert = CreateShader(GL_VERTEX_SHADER, VERT_FILE_DEBUG);
    glAttachShader(prog, vert);
    int geom = CreateShader(GL_GEOMETRY_SHADER, GEOM_FILE_DEBUG);
    glAttachShader(prog, geom);
    int frag = CreateShader(GL_FRAGMENT_SHADER, FRAG_FILE_DEBUG);
    glAttachShader(prog, frag);

    glLinkProgram(prog);
    PrintProgramLog(prog);
    return prog;
}

static int n = -1;
static unsigned int fish_texture;

static int boid_program = -1;
static int boid_program_debug = -1;

void drawBoid(Boid *boid, float timeMs) {
    glPushAttrib(GL_TEXTURE_BIT);
    glPushMatrix();
    {
        lookAt(boid->position, boid->velocity);
        float scale = 0.05f;
        glScaled(scale, scale, scale); // make it small
        glRotatef(180, 0, 1, 0);
        glRotatef(270, 1, 0, 0);

        if (obj == -1) {
            chdir("assets");
            {
                obj = LoadOBJVbo("fish.obj", &n);
                fish_texture = LoadTexBMP("fish.bmp");
            }
            chdir("..");
            boid_program = init_program();
            boid_program_debug = init_program_debug();
        }

        int program = settings.debug ? boid_program_debug : boid_program;
        glUseProgram(program);

        glUniform1i(glGetUniformLocation(program, "show_fog"), settings.adv.fog);
        glUniform1i(glGetUniformLocation(program, "animate_fish"), settings.adv.animate);
        if (settings.debug) {
            glUniform1i(glGetUniformLocation(program, "show_normals"), settings.adv.normals);
        }

        float model_view_matrix[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, model_view_matrix);
        glUniformMatrix4fv(glGetUniformLocation(program, "ModelViewMatrix"), 1, GL_FALSE, model_view_matrix);

        float projection_matrix[16];
        glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);
        glUniformMatrix4fv(glGetUniformLocation(program, "ProjectionMatrix"), 1, GL_FALSE, projection_matrix);

        glPushMatrix();
        {
            glLoadIdentity();
            glTranslatef(boid->position.x, boid->position.y, boid->position.z);
            glScaled(scale, scale, scale); // make it small
            glRotatef(180, 0, 1, 0);
            glRotatef(270, 1, 0, 0);
            float model_matrix[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, model_matrix);
            glUniformMatrix4fv(glGetUniformLocation(program, "ModelMatrix"), 1, GL_FALSE, model_matrix);
        }
        glPopMatrix();

        glUniform3f(glGetUniformLocation(program, "ViewPos"), player.position.x, player.position.y,
                    player.position.z);
        glUniform1f(glGetUniformLocation(program, "time"), timeMs);
        glUniform3f(glGetUniformLocation(program, "LightPos"), 0, 20, 0);
        glUniform4f(glGetUniformLocation(program, "fogColor"), settings.fog_color.x, settings.fog_color.y,
                    settings.fog_color.z, 1);
        glUniform1f(glGetUniformLocation(program, "fogDensity"), settings.fog_density);
        glUniform1f(glGetUniformLocation(program, "fogMax"), settings.fog_linear_max);
        glUniform1f(glGetUniformLocation(program, "fogMin"), settings.fog_linear_min);

        glBindBuffer(GL_ARRAY_BUFFER, obj);

        glEnable(GL_TEXTURE_2D);
        int old;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &old);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, fish_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glUniform1i(glGetUniformLocation(program, "tex"), 6);
        glActiveTexture(old);

        int stride = (3 + 3 + 2) * sizeof(float);

        glVertexAttribPointer(0, 3, GL_FLOAT, 0, stride, (void *) (sizeof(float) * 0));
        glVertexAttribPointer(1, 3, GL_FLOAT, 0, stride, (void *) (sizeof(float) * (3)));
        glVertexAttribPointer(2, 2, GL_FLOAT, 0, stride, (void *) (sizeof(float) * (3 + 3)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glDrawArrays(GL_TRIANGLES, 0, n);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
    glUseProgram(0);
    glPopMatrix();
    glPopAttrib();
}
