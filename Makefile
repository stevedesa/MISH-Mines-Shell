# Compiler
CXX = gcc

# Compiler flags
CXXFLAGS = -Wall -I. -g 

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
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean