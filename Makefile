CC := gcc
CFLAGS := -Wall -Wextra -Wno-unused-result -O2
CFLAGS += -I./core -I./ui -I./solver -I./vendor
LDFLAGS := -L./vendor
LDLIBS := -lraylib -lm -pthread

# Debug flags (use: make DEBUG=1)
ifdef DEBUG
	CFLAGS := -Wall -Wextra -Wno-unused-result -g -O0 \
	          -I./core -I./ui -I./solver -I./vendor
endif

TARGET := cRubik
BENCH_TARGET := bench

SRCS := \
	vendor/raygui.c \
	vendor/cJSON.c \
	solver/kociemba/twoPhase.c \
	solver/kociemba/move.c \
	solver/kociemba/faceCube.c \
	solver/kociemba/enums.c \
	solver/kociemba/cubieCube.c \
	solver/kociemba/coordCube.c \
	solver/solver.c \
	core/average.c \
	core/cublet.c \
	core/cube.c \
	core/keybindings.c \
	core/patterns.c \
	core/playback.c \
	core/queue.c \
	core/timer.c \
	core/scramble.c \
	core/utils.c \
	ui/camera.c \
	ui/options.c \
	ui/ui_cube.c \
	ui/ui_help.c \
	ui/ui_loading.c \
	ui/ui_moves.c \
	ui/ui_patterns.c \
	main.c

BENCH_SRCS := \
	solver/kociemba/twoPhase.c \
	solver/kociemba/move.c \
	solver/kociemba/faceCube.c \
	solver/kociemba/enums.c \
	solver/kociemba/cubieCube.c \
	solver/kociemba/coordCube.c \
	core/cube.c \
	core/cublet.c \
	core/scramble.c \
	core/utils.c \
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
