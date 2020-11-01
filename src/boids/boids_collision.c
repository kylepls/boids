#include "boids_collision.h"
#include "../settings.h"
#include "../world/marching.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "../math/extramath.h"
#include "../lib/CSCIx229.h"
#include "../shaders/shaders.h"
#include "boids_chunks.h"
#include "../window.h"

#define COMP_FILE "src/shaders/triangles.comp"

typedef struct {
    vec3 dir; // the direction of the boid
    float _0;
    vec3 openVelocity;
    float _1;
    vec3 pos; // the position of the boid
    float dist;
} ShaderData;

static const int access_mask_write = GL_MAP_WRITE_BIT;
static const int access_mask_read = GL_MAP_READ_BIT;

static bool init = false;
static unsigned int triangles_shader_prog;
static unsigned int triangles_ssbo;

int make_compute_shader() {
    int prog = glCreateProgram();

    int comp = CreateShader(GL_COMPUTE_SHADER, COMP_FILE);
    glAttachShader(prog, comp);

    glLinkProgram(prog);
    PrintProgramLog(prog);
    return prog;
}

static void init_compute_shader() {
    triangles_shader_prog = make_compute_shader();
    glGenBuffers(1, &triangles_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BOIDS_BUFFER_SIZE * sizeof(ShaderData), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

static void set_shader_uniforms() {
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "iso_level"), settings.iso_level);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "octaves"), settings.octaves);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "persistence"), settings.persistence);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "lacunarity"), settings.lacunarity);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "weight_multiplier"), settings.weight_multiplier);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "noise_offset"), settings.noise_offset);
    glUniform1fv(glGetUniformLocation(triangles_shader_prog, "octave_offsets"), settings.octaves,
                 settings.octave_offsets);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "noise_scale"), settings.noise_scale);
    glUniform1f(glGetUniformLocation(triangles_shader_prog, "world_scale"), settings.world_scale);
}

void apply_boid_collision(Boid *boids, int boids_c, float dt) {
    // slide 27 http://web.engr.oregonstate.edu/~mjb/vulkan/Handouts/OpenglComputeShaders.1pp.pdf
    if (!init) {
        init_compute_shader();
        init = true;
    }

    glEnable(GL_TEXTURE_2D);
    glUseProgram(triangles_shader_prog);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_ssbo);

    ShaderData *data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, boids_c * sizeof(ShaderData), access_mask_write);
    for (int i = 0; i < boids_c; i++) {
        Boid *boid = &boids[i];
        data[i].pos = boid->position;
        data[i].dir = boid->velocity;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    set_shader_uniforms();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, triangles_ssbo);
    glDispatchCompute(boids_c, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, boids_c * sizeof(ShaderData), access_mask_read);

    for (int i = 0; i < boids_c; i++) {
        Boid *boid = &boids[i];
        data[i].pos = boid->position;
        data[i].dir = boid->velocity;

        vec3 open = data[i].openVelocity;
        if (open.x != 0 && open.y != 0 && open.z != 0) {
            boid->velocity = lerpv3(boid->velocity, open, dt * 10);
        }
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
}
