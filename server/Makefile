# ============================================================================
#                    HIGHER LOWER GAME - SERVER BUILD
# ============================================================================
# 
# Cấu trúc modules:
#   main.c          - Server entry point, globals, TCP loop
#   router.c        - HTTP request routing
#   sse.c           - Server-Sent Events handling
#   http.c          - HTTP response utilities
#   database.c      - Game database loading
#   room_init.c     - Room system globals + initialization
#   room_helpers.c  - Helper functions & JSON builders
#   room_handlers.c - Room CRUD handlers
#   game_handlers.c - Game flow handlers
#
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

# Source files - NEW modular structure
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/router.c \
          $(SRC_DIR)/sse.c \
          $(SRC_DIR)/http.c \
          $(SRC_DIR)/database.c \
          $(SRC_DIR)/room_init.c \
          $(SRC_DIR)/room_helpers.c \
          $(SRC_DIR)/room_handlers.c \
          $(SRC_DIR)/game_handlers.c

# Header files
HEADERS = $(INC_DIR)/game.h \
          $(INC_DIR)/config.h \
          $(INC_DIR)/types.h \
          $(INC_DIR)/server.h \
          $(INC_DIR)/sse.h \
          $(INC_DIR)/http.h \
          $(INC_DIR)/room.h \
          $(INC_DIR)/database.h \
          $(INC_DIR)/room_helpers.h

# Object files - matching new source files
OBJECTS = $(OBJ_DIR)/main.o \
          $(OBJ_DIR)/router.o \
          $(OBJ_DIR)/sse.o \
          $(OBJ_DIR)/http.o \
          $(OBJ_DIR)/database.o \
          $(OBJ_DIR)/room_init.o \
          $(OBJ_DIR)/room_helpers.o \
          $(OBJ_DIR)/room_handlers.o \
          $(OBJ_DIR)/game_handlers.o

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
