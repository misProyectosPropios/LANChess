CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -g -I src/common -I src/DataStructures
BUILD_DIR := build

DS_OBJS := $(BUILD_DIR)/ArrayList.o $(BUILD_DIR)/Set.o
COMMON_OBJS := $(BUILD_DIR)/app_info.o $(BUILD_DIR)/board.o $(DS_OBJS)

CLIENT_BIN := $(BUILD_DIR)/lanchess-client
SERVER_BIN := $(BUILD_DIR)/lanchess-server
TEST_BIN := $(BUILD_DIR)/smoke-test

# Test target for DataStructures
DS_TEST_BIN := $(BUILD_DIR)/datastructures-test

# Debug target for DataStructures
debug-dsa: $(DS_TEST_BIN)
	gdb --args ./$(DS_TEST_BIN)

# Search paths for source and header files
vpath %.c src/common src/DataStructures src/client src/server tests
vpath %.h src/common src/DataStructures

.PHONY: all clean run-client run-server test datastructures

all: datastructures $(CLIENT_BIN) $(SERVER_BIN)

# Specific target to generate data structure objects
datastructures: $(DS_OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Universal rule to compile object files from any directory in vpath
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_BIN): src/client/main.c $(COMMON_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(CLIENT_BIN)

$(SERVER_BIN): src/server/main.c $(COMMON_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(SERVER_BIN)

$(TEST_BIN): tests/smoke_test.c $(COMMON_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(TEST_BIN)

$(DS_TEST_BIN): tests/ArrayListTest.c $(DS_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(DS_TEST_BIN)

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

test-datastructures: $(DS_TEST_BIN)
	./$(DS_TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

debug: $(TEST_BIN)
	gdb $(TEST_BIN)

debug-client: $(CLIENT_BIN)
	gdb $(CLIENT_BIN)
