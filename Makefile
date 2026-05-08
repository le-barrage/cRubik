CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-result -O2
CFLAGS += -I./include -I./kociemba
LDFLAGS := -L./include
LDLIBS := -lraylib -lm -pthread

# Debug flags (use: make DEBUG=1)
ifdef DEBUG
	CFLAGS := -Wall -Wextra -Wno-unused-result -g -O0 -I./include -I./kociemba
endif

TARGET := cRubik
BENCH_TARGET := bench

SRCS := \
	include/raygui.c \
	include/cJSON.c \
	kociemba/twoPhase.c \
	kociemba/move.c \
	kociemba/faceCube.c \
	kociemba/enums.c \
	kociemba/cubieCube.c \
	kociemba/coordCube.c \
	average.c \
	camera.c \
	cublet.c \
	cube.c \
	keybindings.c \
	options.c \
	patterns.c \
	playback.c \
	queue.c \
	timer.c \
	scramble.c \
	solver.c \
	ui_cube.c \
	ui_help.c \
	ui_loading.c \
	ui_moves.c \
	ui_patterns.c \
	rubiksCube.c \
	utils.c

BENCH_SRCS := \
	kociemba/twoPhase.c \
	kociemba/move.c \
	kociemba/faceCube.c \
	kociemba/enums.c \
	kociemba/cubieCube.c \
	kociemba/coordCube.c \
	cube.c \
	cublet.c \
	scramble.c \
	utils.c \
	bench.c

BUILDDIR := build
OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)
BENCH_OBJS := $(BENCH_SRCS:%.c=$(BUILDDIR)/%.o)

DEPS := $(OBJS:.o=.d) $(BENCH_OBJS:.o=.d)

.PHONY: all clean run rebuild help bench-run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BENCH_TARGET): $(BENCH_OBJS)
	$(CC) $(BENCH_OBJS) -o $@ $(LDFLAGS) -Wl,--wrap=GetRandomValue $(LDLIBS)

bench-run: $(BENCH_TARGET)
	./$(BENCH_TARGET)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: all
	./$(TARGET)

rebuild: clean all

clean:
	rm -rf $(BUILDDIR) $(TARGET) $(BENCH_TARGET)

help:
	@echo "Usage: make [target] [DEBUG=1]"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build $(TARGET) (default)"
	@echo "  run        - Build and run $(TARGET)"
	@echo "  bench      - Build $(BENCH_TARGET) (solver benchmark)"
	@echo "  bench-run  - Build and run $(BENCH_TARGET)"
	@echo "  clean      - Remove build files"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1    - Build with debug symbols (-g -O0)"
