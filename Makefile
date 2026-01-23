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
	cublet.c \
	cube.c \
	patterns.c \
	queue.c \
	timer.c \
	scramble.c \
	rubiksCube.c \
	utils.c

BUILDDIR := build
OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)

DEPS := $(OBJS:.o=.d)

.PHONY: all clean run rebuild help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: all
	./$(TARGET)

rebuild: clean all

clean:
	rm -rf $(BUILDDIR) $(TARGET)

help:
	@echo "Usage: make [target] [DEBUG=1]"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build $(TARGET) (default)"
	@echo "  run      - Build and run $(TARGET)"
	@echo "  clean    - Remove build files"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1  - Build with debug symbols (-g -O0)"
