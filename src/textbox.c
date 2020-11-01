#include "textbox.h"
#include "lib/CSCIx229.h"
#include "stdbool.h"
#include "settings.h"
#include "player/player.h"

#define MAX_LENGTH 100
static char text[MAX_LENGTH] = "";

bool textbox_open = false;

const vec3 spawn = (vec3) {36.6, 6.8, 19.9};
const float spawnTh = 125;
const float spawnPh = -5.1;

#define COMMANDS 17
char *valid_commands[COMMANDS] = {
        "light", // show occlusion lighting demo
        "normals", // show normals on debug wireframe
        "fog", // toggle world fog
        "fish", // toggle fish
        "caustic_demo", // show caustic demo
        "fps", // show fps top right
        "occlusion", // toggle occlusion calculations
        "animate", // animate fish
        "fish_chunks", // show chunked fish in debug
        "sample_fish", // show example fish at spawn
        "bird", // birds eye view
        "points", // points shader
        "iso", // iso toggle for points
        "spawn", // respawn
        "world", // toggle world draw
        "caustic", // toggle world caustics
        "debug_text" // toggle bottom left debug text
};

static void handle_command() {
    if (strcmp(text, "light") == 0) {
        settings.adv.light_demo ^= 1;
    } else if (strcmp(text, "normals") == 0) {
        settings.adv.normals ^= 1;
    } else if (strcmp(text, "fog") == 0) {
        settings.adv.fog ^= 1;
    } else if (strcmp(text, "fish") == 0) {
        settings.adv.fish ^= 1;
    } else if (strcmp(text, "caustic_demo") == 0) {
        settings.adv.caustic_demo ^= 1;
    } else if (strcmp(text, "fps") == 0) {
        settings.adv.fps ^= 1;
    } else if (strcmp(text, "occlusion") == 0) {
        settings.adv.occlusion ^= 1;
    } else if (strcmp(text, "animate") == 0) {
        settings.adv.animate ^= 1;
    } else if (strcmp(text, "fish_chunks") == 0) {
        settings.adv.fish_chunks ^= 1;
    } else if (strcmp(text, "sample_fish") == 0) {
        settings.adv.sample_fish ^= 1;
    } else if (strcmp(text, "bird") == 0) {
        settings.adv.birds_eye_view ^= 1;
        if (settings.adv.birds_eye_view)
            player.ph = -90;
        else
            player.ph = 0;
    } else if (strcmp(text, "points") == 0) {
        settings.adv.points_shader ^= 1;
    } else if (strcmp(text, "iso") == 0) {
        settings.adv.points_shader_iso ^= 1;
    } else if (strcmp(text, "spawn") == 0) {
        player.position.x = spawn.x;
        player.position.y = spawn.y;
        player.position.z = spawn.z;
        player.th = spawnTh;
        player.ph = spawnPh;
    } else if (strcmp(text, "world") == 0) {
        settings.adv.world ^= 1;
    } else if (strcmp(text, "caustic") == 0) {
        settings.adv.caustic ^= 1;
    } else if (strcmp(text, "debug_text") == 0) {
        settings.adv.debug_text ^= 1;
    } else {
        return;
    }

    toggle_textbox();
}

void toggle_textbox() {
    textbox_open = !textbox_open;
    if (!textbox_open) {
        strncpy(text, "", MAX_LENGTH);
    }
}

void handle_text(char ch) {
    if (ch == 8) {
        text[strlen(text) - 1] = '\0';
    } else if (ch == 13) {
        handle_command();
    } else {
        if (strlen(text) == MAX_LENGTH) { return; }
        char concat[] = {ch, 0};
        strcat(text, concat);
    }
}

void draw_textbox() {
    if (!textbox_open) { return; }

    glColor3f(1, 0, 0);
    for (int i = 0; i < COMMANDS; i++) {
        if (strcmp(text, valid_commands[i]) == 0) {
            glColor3f(0, 1, 0.5);
            break;
        }
    }
    glWindowPos2i(5, 5);
    Print("Command: %s_", text);
}