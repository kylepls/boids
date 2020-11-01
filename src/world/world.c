#include <player/player.h>
#include <caustic/caustic.h>
#include "settings.h"
#include "../lib/CSCIx229.h"
#include "world.h"
#include "math/extramath.h"
#include "../shaders/shaders.h"
#include "../world/marching.h"
#include "../window.h"
#include "math/view_frustum.h"

#define VERT_FILE_NORMAL "src/shaders/world_normal.vert"
#define GEOM_FILE_NORMAL "src/shaders/world_normal.geom"
#define FRAG_FILE_NORMAL "src/shaders/world_normal.frag"

#define VERT_FILE "src/shaders/world.vert"
#define GEOM_FILE "src/shaders/world.geom"
#define FRAG_FILE "src/shaders/world.frag"

#define GEOM_FILE_POINTS "src/shaders/world_points.geom"
#define FRAG_FILE_POINTS "src/shaders/world_points.frag"

unsigned int vertex_buffer_size = 0;
static vec3 *vertex_buffer;

static int world_shader_id;
static int world_shader_id_normal;
static int world_shader_id_points;
static unsigned int tri_table_tex_id;
static unsigned int vertexes_vbo = 0;

int create_world_shader(char *vert_f, char *geom_f, char *frag_f) {
    int prog = glCreateProgram();

    int vert = CreateShader(GL_VERTEX_SHADER, vert_f);
    glAttachShader(prog, vert);
    int geom = CreateShader(GL_GEOMETRY_SHADER, geom_f);
    glAttachShader(prog, geom);
    int frag = CreateShader(GL_FRAGMENT_SHADER, frag_f);
    glAttachShader(prog, frag);

    glLinkProgram(prog);
    PrintProgramLog(prog);
    return prog;
}

void generate_tri_table_texture() {
    glGenTextures(1, &tri_table_tex_id);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glUseProgram(world_shader_id);

    glBindTexture(GL_TEXTURE_2D, tri_table_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA16I_EXT, 16, 256, 0, GL_ALPHA_INTEGER_EXT, GL_INT, &triTable);
    glUniform1i(glGetUniformLocation(world_shader_id, "marching_lookup_table"), 0);

    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}

int get_required_vbo_size() {
    int xz_size = (settings.render_distance * 2) / settings.world_scale;
    int y_size = settings.render_height / settings.world_scale;
    return xz_size * xz_size * y_size;
}

void init_vbo_size() {
    if (vertex_buffer_size != 0) {
        free(vertex_buffer);
    }
    vertex_buffer_size = get_required_vbo_size();
    vertex_buffer = malloc(vertex_buffer_size * sizeof(vec3));
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size * sizeof(vec3), 0, GL_DYNAMIC_DRAW);
}

void init_world() {
    world_shader_id = create_world_shader(VERT_FILE, GEOM_FILE, FRAG_FILE);
    world_shader_id_normal = create_world_shader(VERT_FILE_NORMAL, GEOM_FILE_NORMAL, FRAG_FILE_NORMAL);

    world_shader_id_points = create_world_shader(VERT_FILE_NORMAL, GEOM_FILE_POINTS, FRAG_FILE_POINTS);

    generate_tri_table_texture();

    glGenBuffers(1, &vertexes_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexes_vbo);
    init_vbo_size();
}

void draw_world(float timeMs) {
    if (get_required_vbo_size() != vertex_buffer_size) {
        init_vbo_size();
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_1D);
    unsigned int shader = settings.debug ? world_shader_id_normal : world_shader_id;

    if (settings.adv.points_shader) {
        shader = world_shader_id_points;
    }

    glUseProgram(shader);

    glUniform1i(glGetUniformLocation(shader, "show_fog"), settings.adv.fog);
    if (settings.debug) {
        glUniform1i(glGetUniformLocation(shader, "show_normals"), settings.adv.normals);
    }
    glUniform3f(glGetUniformLocation(shader, "ViewPos"), player.position.x, player.position.y,
                player.position.z);
    glUniform3f(glGetUniformLocation(shader, "LightPos"), 0, 20, 0);
    glUniform4f(glGetUniformLocation(shader, "fogColor"), settings.fog_color.x, settings.fog_color.y,
                settings.fog_color.z, 1);
    glUniform1f(glGetUniformLocation(shader, "fogDensity"), settings.fog_density);
    glUniform1f(glGetUniformLocation(shader, "fogMax"), settings.fog_linear_max);
    glUniform1f(glGetUniformLocation(shader, "fogMin"), settings.fog_linear_min);

    apply_caustic_texture(shader, timeMs);
    glUniform1f(glGetUniformLocation(shader, "caustic_intensity"), settings.caustic_intensity);
    glUniform1f(glGetUniformLocation(shader, "caustic_scale"), settings.caustic_scale);

    float model_view_matrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, model_view_matrix);
    glUniformMatrix4fv(glGetUniformLocation(shader, "ModelViewMatrix"), 1, GL_FALSE, model_view_matrix);

    float projection_matrix[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(shader, "ProjectionMatrix"), 1, GL_FALSE, projection_matrix);

    // marching stuff
    glUniform1i(glGetUniformLocation(shader, "light_demo"), settings.adv.light_demo);
    glUniform1i(glGetUniformLocation(shader, "calculate_occlusion"), settings.adv.occlusion);
    glUniform1i(glGetUniformLocation(shader, "caustic_demo"), settings.adv.caustic_demo);
    glUniform1i(glGetUniformLocation(shader, "show_caustic"), settings.adv.caustic);
    glUniform1f(glGetUniformLocation(shader, "iso_level"), settings.iso_level);
    glUniform1i(glGetUniformLocation(shader, "octaves"), settings.octaves);
    glUniform1f(glGetUniformLocation(shader, "persistence"), settings.persistence);
    glUniform1f(glGetUniformLocation(shader, "lacunarity"), settings.lacunarity);
    glUniform1f(glGetUniformLocation(shader, "weight_multiplier"), settings.weight_multiplier);
    glUniform1f(glGetUniformLocation(shader, "noiseOffset"), settings.noise_offset);
    glUniform1i(glGetUniformLocation(shader, "octaves"), settings.octaves);
    glUniform1fv(glGetUniformLocation(shader, "octave_offsets"), settings.octaves, settings.octave_offsets);
    glUniform1f(glGetUniformLocation(shader, "noise_scale"), settings.noise_scale);
    glUniform1f(glGetUniformLocation(shader, "world_scale"), settings.world_scale);
    glUniform1i(glGetUniformLocation(shader, "points_iso"), settings.adv.points_shader_iso);

    int vertex_n = 0;
    const float scale = settings.world_scale;
    vec3 center_floor = {floorf(player.position.x / scale) * scale, 0, floorf(player.position.z / scale) * scale};
    extractFrustum();

    const float render_distance = settings.render_distance;
    for (float dx = -render_distance; dx <= render_distance; dx += scale) {
        for (float dz = -render_distance; dz <= render_distance; dz += scale) {
            for (float dy = 0; dy < settings.render_height; dy += scale) {
                vec3 v = {center_floor.x + dx, center_floor.y + dy, center_floor.z + dz};
                if (pointInFrustum(v.x, v.y, v.z)) {
                    vertex_buffer[vertex_n++] = v;
                }
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexes_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_n * sizeof(vec3), vertex_buffer);

    // define buffer layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (const void *) 0);

    glDrawArrays(GL_POINTS, 0, vertex_n);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_1D);
}