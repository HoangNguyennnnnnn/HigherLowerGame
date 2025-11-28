# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include
LDFLAGS = -pthread

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/game_server

# Source files
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/http_utils.c $(SRC_DIR)/game_logic.c

# Object files
OBJECTS = $(OBJ_DIR)/main.o $(OBJ_DIR)/http_utils.o $(OBJ_DIR)/game_logic.o

# Default target
all: $(TARGET)

# Create necessary directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Run the server
run: $(TARGET)
	./$(TARGET)

# Rebuild from scratch
rebuild: clean all

.PHONY: all clean run rebuild
