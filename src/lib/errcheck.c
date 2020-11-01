/*
 *  Check for OpenGL errors
 */
#include "CSCIx229.h"

void ErrCheck(const char *where) {
    int err = glGetError();
    if (err) {
        fprintf(stderr, "ERROR %d: %s [%s]\n", err, gluErrorString(err), where);
        fflush(stderr);
    }
}
void ErrCheckf(const char *where, const char* debug) {
    int err = glGetError();
    if (err) {
        fprintf(stderr, "ERROR %d: %s [%s(%s)]\n", err, gluErrorString(err), where, debug);
        fflush(stderr);
    }
}
