CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = backend
OBJ_DIR = obj

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executable name
TARGET = freelancer_matcher

# Default target
all: $(OBJ_DIR) $(TARGET)

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean 