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