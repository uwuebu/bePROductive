# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude
SRC_DIR = code
BIN_DIR = bin
SRC = main.cpp
OBJ = main.o
TARGET = task_tracker.exe

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(BIN_DIR)/$(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(BIN_DIR)/$(OBJ) -Inlohmann_json

# Rule to build object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	del "$(BIN_DIR)\*.o"
	del "$(TARGET)"
