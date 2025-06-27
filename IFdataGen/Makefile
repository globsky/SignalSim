# Universal Makefile for IFdataGen
# Works on Linux, macOS, and Windows (with MSYS2/MinGW)

# Compiler settings
CXX = g++
CC = gcc

# Project name
TARGET = IFdataGen

# Directories
SRCDIR = src
INCDIR = inc
OBJDIR = obj
BINDIR = .

# Source files
SOURCES = $(TARGET).cpp \
          $(SRCDIR)/SatIfSignal.cpp \
          $(SRCDIR)/Almanac.cpp \
          $(SRCDIR)/BCNav1Bit.cpp \
          $(SRCDIR)/BCNav2Bit.cpp \
          $(SRCDIR)/BCNav3Bit.cpp \
          $(SRCDIR)/BCNavBit.cpp \
          $(SRCDIR)/CNav2Bit.cpp \
          $(SRCDIR)/CNavBit.cpp \
          $(SRCDIR)/ComplexNumber.cpp \
          $(SRCDIR)/Coordinate.cpp \
          $(SRCDIR)/D1D2NavBit.cpp \
          $(SRCDIR)/FNavBit.cpp \
          $(SRCDIR)/GNavBit.cpp \
          $(SRCDIR)/GnssTime.cpp \
          $(SRCDIR)/INavBit.cpp \
          $(SRCDIR)/JsonInterpreter.cpp \
          $(SRCDIR)/JsonParser.cpp \
          $(SRCDIR)/LNavBit.cpp \
          $(SRCDIR)/NavBit.cpp \
          $(SRCDIR)/NavData.cpp \
          $(SRCDIR)/PilotBit.cpp \
          $(SRCDIR)/PowerControl.cpp \
          $(SRCDIR)/PrnGenerate.cpp \
          $(SRCDIR)/Rinex.cpp \
          $(SRCDIR)/SatelliteParam.cpp \
          $(SRCDIR)/SatelliteSignal.cpp \
          $(SRCDIR)/Trajectory.cpp \
          $(SRCDIR)/XmlArguments.cpp \
          $(SRCDIR)/XmlElement.cpp \
          $(SRCDIR)/XmlInterpreter.cpp \
          $(SRCDIR)/FastMath.cpp

# Object files
OBJECTS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(notdir $(SOURCES)))

# Compiler flags
CXXFLAGS = -g -O0 -Wall -std=c++11 -I$(INCDIR)
LDFLAGS = -lm

# Check for OpenMP support
OPENMP_TEST := $(shell echo 'int main() { return 0; }' | $(CXX) -fopenmp -x c++ - -o /dev/null 2>/dev/null && echo "yes" || echo "no")
ifeq ($(OPENMP_TEST),yes)
    CXXFLAGS += -fopenmp -DOPENMP_ENABLED
    LDFLAGS += -fopenmp
    $(info OpenMP support: ENABLED)
else
    $(info OpenMP support: DISABLED (install OpenMP for better performance))
endif

# OS-specific settings
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME_S),Linux)
    # Linux specific flags
    CXXFLAGS += -pthread
    LDFLAGS += -pthread
endif
ifeq ($(UNAME_S),Darwin)
    # macOS specific flags
    # On macOS, you might need to specify OpenMP paths explicitly
    # CXXFLAGS += -I/usr/local/opt/libomp/include
    # LDFLAGS += -L/usr/local/opt/libomp/lib
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
    # Windows MinGW specific flags
    TARGET := $(TARGET).exe
endif
ifneq (,$(findstring MSYS,$(UNAME_S)))
    # Windows MSYS2 specific flags
    TARGET := $(TARGET).exe
endif

# Default target
all: $(OBJDIR) $(BINDIR)/$(TARGET)

# Create obj directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Link executable
$(BINDIR)/$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files from current directory
$(OBJDIR)/IFdataGen.o: IFdataGen.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile source files from src directory
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning..."
	@rm -rf $(OBJDIR)
	@rm -f $(BINDIR)/$(TARGET)
	@echo "Clean complete"

# Install (optional)
install: $(BINDIR)/$(TARGET)
	@echo "Installing to /usr/local/bin..."
	@sudo cp $(BINDIR)/$(TARGET) /usr/local/bin/
	@echo "Installation complete"

# Run test
test: $(BINDIR)/$(TARGET)
	@echo "Running test..."
	./$(TARGET) IfGenTest.json

# Help
help:
	@echo "IFdataGen Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make              - Build the project"
	@echo "  make clean        - Remove all build files"
	@echo "  make test         - Build and run test"
	@echo "  make install      - Install to /usr/local/bin (requires sudo)"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Options:"
	@echo "  CXX=clang++       - Use different C++ compiler"
	@echo "  CXXFLAGS=-g       - Add debug symbols"
	@echo ""
	@echo "Example:"
	@echo "  make CXX=clang++ CXXFLAGS='-O3 -march=native'"

# Phony targets
.PHONY: all clean install test help

# Dependencies are handled by implicit rules