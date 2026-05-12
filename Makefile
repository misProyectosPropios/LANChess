CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -I src/common
BUILD_DIR := build

COMMON_SRC := src/common/app_info.c src/common/board.c
CLIENT_SRC := src/client/main.c $(COMMON_SRC)
SERVER_SRC := src/server/main.c $(COMMON_SRC)
TEST_SRC := tests/smoke_test.c $(COMMON_SRC)

CLIENT_BIN := $(BUILD_DIR)/lanchess-client
SERVER_BIN := $(BUILD_DIR)/lanchess-server
TEST_BIN := $(BUILD_DIR)/smoke-test

.PHONY: all clean run-client run-server test

all: $(CLIENT_BIN) $(SERVER_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(CLIENT_BIN): $(CLIENT_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN)

$(TEST_BIN): $(TEST_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
