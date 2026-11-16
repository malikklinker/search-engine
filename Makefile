CC = clang
MODE ?= debug
BUILD := build/$(MODE)
.DEFAULT_GOAL := all
.PHONY: all clean release sanitize test
release:
	$(MAKE) MODE=release all
sanitize:
	$(MAKE) MODE=sanitize all
clean:
	rm -rf build
CFLAGS ?= -Wall -Wextra -Wpedantic -Werror
CFLAGS += -MMD -MP
ifeq ($(MODE),release)
CFLAGS += -O2
else
CFLAGS += -g -O0
endif
ifeq ($(MODE),sanitize)
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif
LDLIBS += -lm

all: $(BUILD)/searchengine $(BUILD)/index_lib_test $(BUILD)/string_set_test $(BUILD)/string_list_test

$(BUILD)/searchengine: $(BUILD)/main.o $(BUILD)/index_lib.o $(BUILD)/string_list.o $(BUILD)/string_set.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/index_lib_test: $(BUILD)/index_lib_test.o $(BUILD)/index_lib.o $(BUILD)/string_list.o $(BUILD)/string_set.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/string_set_test: $(BUILD)/string_set_test.o $(BUILD)/string_set.o $(BUILD)/string_list.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/string_list_test: $(BUILD)/string_list_test.o $(BUILD)/string_list.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: all $(BUILD)/regression_test
	./$(BUILD)/regression_test
	./$(BUILD)/index_lib_test
	./$(BUILD)/string_set_test
	./$(BUILD)/string_list_test

-include $(wildcard $(BUILD)/*.d $(BUILD)/*/*.d)

$(BUILD)/regression_test: $(BUILD)/regression_test.o $(BUILD)/index_lib.o $(BUILD)/string_list.o $(BUILD)/string_set.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
