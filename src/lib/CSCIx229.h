#ifndef CSCIx229
#define CSCIx229

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32 // ignoring the USEGLEW macro in favor of this, CLion won't pick up the useglew macro
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define Cos(th) cosf(3.1415926/180*(th))
#define Sin(th) sinf(3.1415926/180*(th))

#ifdef __cplusplus
extern "C" {
#endif

void Print(const char* format , ...);
void Fatal(const char* format , ...);
unsigned int LoadTexBMP(const char* file);
void Project(double fov,double asp,double dim);
void ErrCheck(const char* where);
void ErrCheckf(const char *where, const char* debug);
int  LoadOBJ(const char* file);
unsigned int LoadOBJVbo(const char *file, int *n);

#ifdef __cplusplus
}
#endif

#endif
