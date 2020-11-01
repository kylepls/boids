#include <caustic/caustic.h>
#include "window.h"
#include "lib/CSCIx229.h"
#include "boids/boids.h"
#include "boids/boids_chunks.h"
#include "settings.h"
#include "player/player.h"
#include "textbox.h"

#define WINDOW_TITLE "Boids"

int controlling_cursor = 0;

int last_window_x = -1;
int last_window_y = -1;

void bound_view_angle() {
    while (player.th > 360) player.th -= 360;
    while (player.th < 0) player.th += 360;
    if (player.ph < -90) {
        player.ph = -90;
    }
    if (player.ph > 90) {
        player.ph = 90;
    }
}


void project() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(settings.field_of_view, settings.aspect_ratio, settings.dim / 256, 16 * settings.dim);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void special(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) {
        player.th += 5;
    } else if (key == GLUT_KEY_LEFT) {
        player.th -= 5;
    } else if (key == GLUT_KEY_UP) {
        player.ph += 5;
    } else if (key == GLUT_KEY_DOWN) {
        player.ph -= 5;
    }
    bound_view_angle();
    project();
    glutPostRedisplay();
}

void key_up(unsigned char ch, int x, int y) {
    if (ch == 'w') {
        player.acceleration.x = 0;
    } else if (ch == 'a') {
        player.acceleration.z = 0;
    } else if (ch == 's') {
        player.acceleration.x = 0;
    } else if (ch == 'd') {
        player.acceleration.z = 0;
    }
}

void key(unsigned char ch, int x, int y) {
    if (textbox_open) {
        if (ch == 27) {
            toggle_textbox();
        } else {
            handle_text(ch);
        }
    } else if (ch == 'w') {
        player.acceleration.x = 1;
    } else if (ch == 'a') {
        player.acceleration.z = 1;
    } else if (ch == 's') {
        player.acceleration.x = -1;
    } else if (ch == 'd') {
        player.acceleration.z = -1;
    } else if (ch == 'p') {
        settings.pause ^= true;
    } else if (ch == 't' && !textbox_open) {
        toggle_textbox();
    } else if (ch == 27) {
        exit(0);
    } else if (ch == '1') {
        settings.boids.cohesion_radius += 0.1;
    } else if (ch == '!') {
        settings.boids.cohesion_radius -= 0.1;
    } else if (ch == '2') {
        settings.boids.cohesion_multiplier += 0.1;
    } else if (ch == '@') {
        settings.boids.cohesion_multiplier -= 0.1;
    } else if (ch == '3') {
        settings.boids.separation_radius += 0.1;
    } else if (ch == '#') {
        settings.boids.separation_radius -= 0.1;
    } else if (ch == '4') {
        settings.boids.separation_multiplier += 0.1;
    } else if (ch == '$') {
        settings.boids.separation_multiplier -= 0.1;
    } else if (ch == '5') {
        settings.boids.alignment_radius += 0.1;
    } else if (ch == '%') {
        settings.boids.alignment_radius -= 0.1;
    } else if (ch == '6') {
        settings.boids.alignment_multiplier += 0.1;
    } else if (ch == '^') {
        settings.boids.alignment_multiplier -= 0.1;
    } else if (ch == '7') {
        settings.boids.max_velocity += 0.1;
    } else if (ch == '&') {
        settings.boids.max_velocity -= 0.1;
    } else if (ch == '8') {
        settings.fog_density += 0.01;
    } else if (ch == '*') {
        settings.fog_density -= 0.01;
    } else if (ch == '9') {
        settings.caustic_intensity += 0.01;
    } else if (ch == '(') {
        settings.caustic_intensity -= 0.01;
    } else if (ch == '0') {
        settings.caustic_scale += 0.1;
    } else if (ch == ')') {
        settings.caustic_scale -= 0.1;
    } else if (ch == 'x') {
        settings.debug ^= true;
    } else if (ch == 'C') {
        settings.iso_level--;
    } else if (ch == 'c') {
        settings.iso_level++;
    } else if (ch == 'V') {
        settings.octaves = fmax(settings.octaves - 1, 3);
    } else if (ch == 'v') {
        settings.octaves = fmin(settings.octaves + 1, 17);
    } else if (ch == 'b') {
        settings.persistence = fmin(settings.persistence + 0.05, 1);
    } else if (ch == 'B') {
        settings.persistence = fmax(settings.persistence - 0.05, 0.1);
    } else if (ch == 'n') {
        settings.lacunarity = fmin(settings.lacunarity + 0.05, 10);
    } else if (ch == 'N') {
        settings.lacunarity = fmax(settings.lacunarity - 0.05, 0.1);
    } else if (ch == '.') {
        settings.noise_offset = rand() * 1.239892387;
    } else if (ch == 'l') {
        settings.drawWorld ^= 1;
    }

    project();
    glutPostRedisplay();
}

void reshape(int width, int height) {
    settings.aspect_ratio = (height > 0) ? (double) width / height : 1;
    glViewport(0, 0, width, height);
    project();
}

void motion(int x, int y) {
    if (!controlling_cursor) {
        glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
    } else {
        if (x != last_window_x || y != last_window_y) {
            glutSetCursor(GLUT_CURSOR_NONE);
            int size_x = glutGet(GLUT_WINDOW_WIDTH);
            int size_y = glutGet(GLUT_WINDOW_HEIGHT);

            double sensitivity = 3;

            int midX = size_x >> 1;
            int midY = size_y >> 1;

            glutWarpPointer(midX, midY);

            player.th -= (midX - x) / 100.0 * sensitivity;
            player.ph += (midY - y) / 100.0 * sensitivity;

            bound_view_angle();
            glutPostRedisplay();
            last_window_x = x;
            last_window_y = y;
        }
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && GLUT_DOWN == state) { controlling_cursor = (controlling_cursor + 1) % 2; }
}

void setup_window(void (*func)()) {
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800, 800);
    glutCreateWindow(WINDOW_TITLE);

#ifdef USEGLEW
    if (glewInit() != GLEW_OK) { Fatal("Error initializing GLEW\n"); }
    printf("Using GLEW: %s\n", glewGetString(GLEW_VERSION));
    fflush(stdout);
#endif

    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);
    glutKeyboardFunc(key);
    glutKeyboardUpFunc(key_up);
    glutIdleFunc(idle);
    glutDisplayFunc(func);
    glutMouseFunc(mouse);

    init_caustic_textures();
    ErrCheck("init");
    glutMainLoop();
}
