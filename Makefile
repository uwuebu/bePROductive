# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude
SRC_DIR = code
BIN_DIR = bin
SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/task.cpp
OBJ = $(BIN_DIR)/main.o $(BIN_DIR)/task.o
TARGET = task_tracker.exe

# Default target
all: $(BIN_DIR) $(TARGET)

# Rule to build the target
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) -Inlohmann_json

# Rule to build object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to ensure the bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up build files
clean:
	del "$(BIN_DIR)\*.o"
	del "$(TARGET)"
