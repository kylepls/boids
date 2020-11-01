#include "caustic.h"
#include "../lib/CSCIx229.h"
#include "stdbool.h"
#include "math.h"

#define TEX_C 64

const float caustic_loop = 64.0 / 25.0 * 1000;
static unsigned int caustics[TEX_C];

void init_caustic_textures() {
    char *str = (char *) malloc(100 * sizeof(char));
    for (int i = 0; i < TEX_C; ++i) {
        sprintf(str, "assets/caustic/%d.bmp", i + 1);
        caustics[i] = LoadTexBMP(str);
    }
    ErrCheck("init_caustic_textures");
}

void apply_caustic_texture(unsigned int program, float timeMs) {
    int old;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &old);

    int index = round(fmod(timeMs, caustic_loop) / (caustic_loop / (TEX_C - 1)));

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, caustics[index]);
    glUniform1i(glGetUniformLocation(program, "caustic"), 5);

    glActiveTexture(old);
}