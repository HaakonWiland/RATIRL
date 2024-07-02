# Spesify the compiler 
CXX = g++

# Compiler flags: C++ standard to C++20, Wall enable all warnings 
CXXFLAGS = -std=c++20 -static -static-libgcc -static-libstdc++ -Wall  

# FLags passed to the linker
LDFLAGS = -static

# Target executable 
TARGET = keylogger.exe 

# Source files and object files
SRCS = keylogger.cpp network.cpp
OBJS = $(SRCS:.cpp=.o)

# Define include directories:
# INCLUDES = -I"C:/Users/thelab/development/RAT/include"

DLL_DIR = C:\\msys64\\ucrt64\\bin
DLLS = libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll


# Default target that gets built when we run make 
all: $(TARGET) copy_dlls

# Linking step: define how we build the .exe from the .o files. 
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) -lws2_32

# Compilation step for .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

copy_dlls:
	@echo "Copying necessary DLLs..."
	copy "$(DLL_DIR)\\libstdc++-6.dll" .
	copy "$(DLL_DIR)\\libgcc_s_seh-1.dll" .
	copy "$(DLL_DIR)\\libwinpthread-1.dll" .


# Clean target to remove obj files and executable. (NOTE Linux is different then Windows here)
clean:
	del /f /q $(OBJS) $(TARGET)

# Clean as phony to avoid conflicts with files named clean 
.PHONY: clean




