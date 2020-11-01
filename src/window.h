#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include "stdbool.h"
#include "math/extramath.h"
#include "lib/CSCIx229.h"

void setup_window(void (*func)());

void display();

void idle();

#endif //TEST_WINDOW_H
