CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -O2

# Admitir el proyecto organizado y la copia plana de entrega.
ifneq ($(wildcard src/ratmaze.c),)
SRC_DIR = src
TEST_DIR = tests
else
SRC_DIR = .
TEST_DIR = .
endif
BIN_DIR = output/bin
SOURCES = $(addprefix $(SRC_DIR)/,ratmaze.c maze.c solver.c timer.c)
HEADERS = $(addprefix $(SRC_DIR)/,maze.h solver.h timer.h)

.PHONY: all test clean
all: $(BIN_DIR)/ratmaze $(BIN_DIR)/generate_maze

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/ratmaze: $(SOURCES) $(HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(SOURCES) -o $@

$(BIN_DIR)/generate_maze: $(SRC_DIR)/generate_maze.c $(SRC_DIR)/maze.c $(SRC_DIR)/maze.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(SRC_DIR)/generate_maze.c $(SRC_DIR)/maze.c -o $@

$(BIN_DIR)/test_ratmaze: $(TEST_DIR)/test_ratmaze.c $(SOURCES) $(HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(TEST_DIR)/test_ratmaze.c -o $@

test: $(BIN_DIR)/test_ratmaze
	./$(BIN_DIR)/test_ratmaze

clean:
	rm -f $(BIN_DIR)/ratmaze $(BIN_DIR)/generate_maze $(BIN_DIR)/test_ratmaze $(BIN_DIR)/ratmaze.exe $(BIN_DIR)/generate_maze.exe $(BIN_DIR)/test_ratmaze.exe
