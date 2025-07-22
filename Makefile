BUILD_DIR	:= build
CFLAGS		:= -g3 -Wmaybe-uninitialized -Wall -Wextra
LDFLAGS		:= -lZydis
CC		:= gcc

all: $(BUILD_DIR)/child $(BUILD_DIR)/tracer
test: $(BUILD_DIR)/child $(BUILD_DIR)/tracer
	$(BUILD_DIR)/tracer --file $(BUILD_DIR)/child

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILD_DIR)/child: child.asm | $(BUILD_DIR)
	fasm $^ $@
	chmod +x $@

$(BUILD_DIR)/tracer: $(BUILD_DIR)/tracer.o | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@
