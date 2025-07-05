# Simple Makefile for ASCII Tetris

# Compiler
CXX = g++
# Output binary name
TARGET = tetris
# Source files
SRC = main.cpp
# Compiler flags
CXXFLAGS = -Wall -std=c++17

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
