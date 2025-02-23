# Compiler
CXX = g++

# Source files
SRCS = main.cpp helper.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = mish

# Default rule
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean

# If you want to compile with gcc, here's the commented out code for that
# CXX = gcc
# CXXFLAGS = -std=c++17 -Wall -Wextra -O2
# LDFLAGS = -lstdc++
# TARGET = my_program
# SRC = main.cpp utils.cpp
# OBJ = $(SRC:.cpp=.o)

# $(TARGET): $(OBJ)
# 	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJ) $(TARGET)