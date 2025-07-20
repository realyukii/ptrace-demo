BUILD_DIR := build

all: $(BUILD_DIR)/child $(BUILD_DIR)/tracer
	$(BUILD_DIR)/tracer $(BUILD_DIR)/child

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/child: child.c | $(BUILD_DIR)
	cc $^ -o $@

$(BUILD_DIR)/tracer: tracer.c | $(BUILD_DIR)
	cc $^ -o $@
