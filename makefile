CXXCOMPILER = g++ -std=c++11
TARGET := x_engine
TEST_TARGET := x_test
RM := rm -rf
TEST_SRC := test
SRC_DIR := src
INC_DIR := inc 
BUILD_DIR := release
SRC_EXT := cpp
BINARY_DIR = $(BUILD_DIR)/obj


OCL_INC  = -I/opt/AMDAPPSDK-3.0/include/
OCL_LIB  = -L/usr/lib64/OpenCL/vendors/amd/

LIBS := -lOpenCL
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	LIBS := -framework OpenCL
endif


SRC = $(wildcard $(SRC_DIR)/*.$(SRC_EXT))

OBJ := $(patsubst $(SRC_DIR)/%,$(BINARY_DIR)/%,$(SRC:.$(SRC_EXT)=.o))

CXXFLAGS = $(CXXCOMPILER)

all: $(TARGET)
all: CXXFLAGS += -O3 -Wall

debug: CXXFLAGS += -ggdb -O0
debug: $(TARGET)

test: CXXFLAGS += -framework boost
test: $(TARGET)

$(TARGET): $(OBJ)
	@echo 'Building target: $@'
	$(CXXFLAGS) $(OCL_LIB) -o $(BUILD_DIR)/$(TARGET) $(OBJ) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(BINARY_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BINARY_DIR)
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXXFLAGS) $(OCL_INC) -I$(INC_DIR)  -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

clean:
	$(RM) $(BUILD_DIR)

.PHONY: all clean debug mac_os
