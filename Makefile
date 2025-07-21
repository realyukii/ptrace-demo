BUILD_DIR	:= build
CFLAGS		:= -Wmaybe-uninitialized -Wall -Wextra
CC		:= gcc

all: $(BUILD_DIR)/child $(BUILD_DIR)/tracer
	$(BUILD_DIR)/tracer $(BUILD_DIR)/child

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/child: child.asm | $(BUILD_DIR)
	fasm $^ $@
	chmod +x $@

$(BUILD_DIR)/tracer: tracer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@
