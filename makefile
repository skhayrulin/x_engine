CXXCOMPILER = g++ -std=c++11
TARGET := x_engine
RM := rm -rf
SRC_DIR := src
INC_DIR := inc 
BUILD_DIR := release
SRC_EXT := cpp
BINARY_DIR = $(BUILD_DIR)/obj

LIBS := -lpython2.7 -lGL -lGLU -lOpenCL -lrt -lglut

SRC = $(wildcard $(SRC_DIR)/*.$(SRC_EXT))

OBJ := $(patsubst $(SRC_DIR)/%,$(BINARY_DIR)/%,$(SRC:.$(SRC_EXT)=.o))

CXXFLAGS = $(CXXCOMPILER)

all : $(TARGET)
all:  CXXFLAGS += -O3 -Wall 

debug: CXXFLAGS += -ggdb -O0
debug: $(TARGET)


$(TARGET): $(OBJ)
	@echo 'Building target: $@'
	$(CXXFLAGS) -L/usr/lib64/OpenCL/vendors/amd/ -o $(BUILD_DIR)/$(TARGET) $(OBJ) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(BINARY_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BINARY_DIR)
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXXFLAGS) -I/usr/include/python2.7 -I/opt/AMDAPPSDK-3.0/include/ -I$(INC_DIR)  -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean:
	$(RM) $(BUILD_DIR)

.PHONY: all clean debug
