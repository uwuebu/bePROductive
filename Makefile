# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude -Isqlite  # Include directories for headers
SRC_DIR = code
BIN_DIR = bin
SQLITE_LIB_DIR = sqlite  # Directory containing sqlite3.def and sqlite3.dll
SQLITE_LIB = -lsqlite3   # SQLite library to link against

# Source and object files
SRC = $(SRC_DIR)/main.cpp $(SRC_DIR)/database.cpp
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
TARGET = bePROductive.exe

# Default target
all: $(BIN_DIR) $(TARGET)

# Rule to build the target
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(SQLITE_LIB) -L$(SQLITE_LIB_DIR)

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
