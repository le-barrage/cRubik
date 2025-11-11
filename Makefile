CC := gcc
CFLAGS := -Wall -Wextra -O2
DEBUGFLAGS := -g -O0

SRCS := \
	include/raygui.c \
	kociemba/twoPhase.c \
	kociemba/move.c \
	kociemba/faceCube.c \
	kociemba/enums.c \
	kociemba/cubieCube.c \
	kociemba/coordCube.c \
	average.c \
	cublet.c \
	cube.c \
	patterns.c \
	queue.c \
	timer.c \
	scramble.c \
	rubiksCube.c \
	utils.c

TARGET := cRubik

LDLIBS := -L ./include/ -lraylib -lm -pthread

.PHONY: all clean run debug help

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDLIBS)

debug: CFLAGS := $(CFLAGS) $(DEBUGFLAGS)
debug: clean $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

help:
	@echo "TARGET = $(TARGET)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "LDLIBS = $(LDLIBS)"
