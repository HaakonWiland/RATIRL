# Spesify the compiler 
CXX = g++

# Compiler flags: C++ standard to C++11, Wall enable all warnings 
CXXFLAGS = -std=c++11 -Wall 

# Target executable 
TARGET = keylogger.exe 

# Source files and object files
SRCS = main.cpp network.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target: When we run "make" we will build "all"? 
all: $(TARGET)

# Linking step 
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) -lws2_32

# Compilation step for .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove obj files and executable. (NOTE Linux is different then Windows here)
clean:
	del /f /q $(OBJS) $(TARGET)

# Clean as phony to avoid conflicts with files named clean 
.PHONY: clean




