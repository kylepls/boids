EXE=main

all: $(EXE)

SRC_DIR=./src
SRC=$(wildcard src/*.c) $(wildcard src/**/*.c)

CFLAGS = -I${SRC_DIR}

ifeq "$(OS)" "Windows_NT"
CFLAGS += -O3 -Wall -DUSEGLEW
LIBS +=-lglew32 -lfreeglut -lglu32 -lopengl32
CLEAN+=rm -f *.exe *.o *.a src/lib/*.o src/lib/*.a
else
ifeq "$(shell uname)" "Darwin"
CFLAGS += -O3 -Wall -Wno-deprecated-declarations
LIBS += -framework GLUT -framework OpenGL
else
CFLAGS += -O3 -Wall
LIBS += -lglut -lGLU -lGL -lm
endif
CLEAN=rm -f $(EXE) *.o *.a
endif

main: $(SRC)
	gcc $(CFLAGS) -O3 -o $@ $^   $(LIBS)

clean:
	$(CLEAN)
