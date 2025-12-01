# ============================================================================
#                    HIGHER LOWER GAME - SERVER BUILD
# ============================================================================

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
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/http_utils.c \
          $(SRC_DIR)/game_logic.c \
          $(SRC_DIR)/room_logic.c

# Header files
HEADERS = $(INC_DIR)/game.h \
          $(INC_DIR)/config.h \
          $(INC_DIR)/types.h \
          $(INC_DIR)/server.h \
          $(INC_DIR)/sse.h \
          $(INC_DIR)/http.h \
          $(INC_DIR)/room.h \
          $(INC_DIR)/game_logic.h

# Object files
OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/http_utils.o \
          $(OBJ_DIR)/game_logic.o \
          $(OBJ_DIR)/room_logic.o

# Default target
all: $(TARGET)

# Create necessary directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile source files to object files (depend on all headers)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✅ Build complete: $(TARGET)"
	@echo ""

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "🧹 Clean complete"

# Run the server
run: $(TARGET)
	./$(TARGET)

# Rebuild from scratch
rebuild: clean all

# Show help
help:
	@echo "Higher Lower Game Server - Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build the server (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  run      - Build and run the server"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  help     - Show this help"

.PHONY: all clean run rebuild help

.PHONY: all clean run rebuild
