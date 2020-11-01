#include "shaders.h"

#include "../lib/CSCIx229.h"
#include "stdio.h"
#include "stdlib.h"

char *ReadText(char *file) {
    int n;
    char *buffer;
    FILE *f = fopen(file, "rt");
    if (!f) Fatal("Cannot open text file %s\n", file);
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    rewind(f);
    buffer = (char *) malloc(n + 1);
    if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n", n + 1, file);
    if (fread(buffer, n, 1, f) != 1) Fatal("Cannot read %d bytes for text file %s: %s\n", n, file, buffer);
    buffer[n] = 0;
    fclose(f);
    return buffer;
}

int PrintShaderLog(int obj, char *file) {
    int len = 0;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        int n = 0;
        char *buffer = (char *) malloc(len);
        if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n", len);
        glGetShaderInfoLog(obj, len, &n, buffer);
        fprintf(stderr, "%s:\n%s\n", file, buffer);
        free(buffer);
    }
    glGetShaderiv(obj, GL_COMPILE_STATUS, &len);
    if (!len) {
        Fatal("Error compiling %s\n", file);
        return 0;
    } else {
        return 1;
    }
}

int PrintProgramLog(int obj) {
    int len = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        int n = 0;
        char *buffer = (char *) malloc(len);
        if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n", len);
        glGetProgramInfoLog(obj, len, &n, buffer);
        fprintf(stderr, "%s\n", buffer);
    }
    glGetProgramiv(obj, GL_LINK_STATUS, &len);
    if (!len) {
        Fatal("Error linking program\n");
        return 0;
    } else {
        return 1;
    }
}

int CreateShader(GLenum type, char *file) {
    int shader = glCreateShader(type);
    char *source = ReadText(file);
    glShaderSource(shader, 1, (const char **) &source, NULL);
    free(source);
    fprintf(stderr, "Compile %s\n", file);
    glCompileShader(shader);
    if (PrintShaderLog(shader, file)) {
        return shader;
    } else {
        return -1;
    }
}
