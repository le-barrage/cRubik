CC := gcc

BUILDDIR := build
RAYLIB_SRC := vendor/raylib-6.0/src
RAYLIB_LIB := $(BUILDDIR)/raylib/libraylib.a

# Platform-specific bits. $(OS) is "Windows_NT" only in MSYS2/MinGW shells.
ifeq ($(OS),Windows_NT)
	EXE_EXT       := .exe
	PLATFORM_LIBS := -lopengl32 -lgdi32 -lwinmm
else
	EXE_EXT       :=
	PLATFORM_LIBS := -lGL -ldl -lrt -lX11
endif

CFLAGS := -Wall -Wextra -Wno-unused-result -O2
CFLAGS += -I./core -I./ui -I./solver -isystem ./vendor -isystem $(RAYLIB_SRC)
LDFLAGS := -L$(dir $(RAYLIB_LIB))
LDLIBS := -lraylib $(PLATFORM_LIBS) -lm -lpthread

# Debug flags (use: make DEBUG=1)
ifdef DEBUG
	CFLAGS := -Wall -Wextra -Wno-unused-result -g -O0 \
	          -I./core -I./ui -I./solver -isystem ./vendor -isystem $(RAYLIB_SRC)
endif

TARGET := cRubik$(EXE_EXT)
BENCH_TARGET := bench$(EXE_EXT)

SRCS := \
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
	core/logger.c \
	core/patterns.c \
	core/playback.c \
	core/queue.c \
	core/timer.c \
	core/scramble.c \
	core/utils.c \
	ui/camera.c \
	ui/font.c \
	ui/options.c \
	ui/ui_cube.c \
	ui/ui_help.c \
	ui/ui_loading.c \
	ui/ui_moves.c \
	ui/ui_patterns.c \
	ui/widgets.c \
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
	core/logger.c \
	core/scramble.c \
	core/utils.c \
	bench.c

OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)
BENCH_OBJS := $(BENCH_SRCS:%.c=$(BUILDDIR)/%.o)

DEPS := $(OBJS:.o=.d) $(BENCH_OBJS:.o=.d)

.PHONY: all clean run rebuild help bench-run raylib raylib-clean

all: $(TARGET)

$(TARGET): $(OBJS) $(RAYLIB_LIB)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BENCH_TARGET): $(BENCH_OBJS) $(RAYLIB_LIB)
	$(CC) $(BENCH_OBJS) -o $@ $(LDFLAGS) -Wl,--wrap=GetRandomValue $(LDLIBS)

bench-run: $(BENCH_TARGET)
	./$(BENCH_TARGET)

$(RAYLIB_LIB):
	@mkdir -p $(dir $@)
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC \
	  RAYLIB_RELEASE_PATH=$(abspath $(dir $(RAYLIB_LIB)))

raylib: $(RAYLIB_LIB)

raylib-clean:
	-$(MAKE) -C $(RAYLIB_SRC) clean

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: all
	./$(TARGET)

rebuild: clean all

clean: raylib-clean
	rm -rf $(BUILDDIR) $(TARGET) $(BENCH_TARGET)

help:
	@echo "Usage: make [target] [DEBUG=1]"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build $(TARGET) (default)"
	@echo "  run          - Build and run $(TARGET)"
	@echo "  bench        - Build $(BENCH_TARGET) (solver benchmark)"
	@echo "  bench-run    - Build and run $(BENCH_TARGET)"
	@echo "  raylib       - Build raylib into $(RAYLIB_LIB)"
	@echo "  raylib-clean - Remove raylib build artefacts (in $(RAYLIB_SRC))"
	@echo "  clean        - Remove all build files (including raylib)"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1    - Build with debug symbols (-g -O0)"
