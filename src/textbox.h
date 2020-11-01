#ifndef PROJECT_TEXTBOX_H
#define PROJECT_TEXTBOX_H

#include "stdbool.h"

void draw_textbox();

extern bool textbox_open;

void handle_text(char ch);

void toggle_textbox();

#endif //PROJECT_TEXTBOX_H
