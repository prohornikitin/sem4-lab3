CPP.COMPILER ?= g++
BUILD ?= debug

DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
A_SRC_DIR := $(DIR)/src/A
B_SRC_DIR := $(DIR)/src/B
A_INCLUDE_DIR := $(DIR)/src/A
B_INCLUDE_DIR := $(DIR)/src/B
BUILD_DIR := $(DIR)/build/$(BUILD)

A_EXECUTABLE := $(BUILD_DIR)/A
B_EXECUTABLE := $(BUILD_DIR)/B

CPP.FLAGS := -pedantic -Wall -Wextra -std=gnu++20 -lreadline
ifeq ($(BUILD),release)
	CPP.FLAGS += -Ofast
else
	CPP.FLAGS += -g -O0 -D DEBUG
endif

A_SRC := $(shell find $(A_SRC_DIR) -name '*.cpp')
B_SRC := $(shell find $(B_SRC_DIR) -name '*.cpp')
A_HEADERS := $(shell find $(A_INCLUDE_DIR) -name '*.h')
B_HEADERS := $(shell find $(B_INCLUDE_DIR) -name '*.h')

all: $(A_EXECUTABLE)

run-a: $(A_EXECUTABLE)
	exec $(A_EXECUTABLE) $(B_EXECUTABLE) 2

run-b: $(B_EXECUTABLE)
	exec $(B_EXECUTABLE)

run-a-vg: $(A_EXECUTABLE)
	valgrind --leak-check=full $(A_EXECUTABLE) $(B_EXECUTABLE)

run-b-vg: $(B_EXECUTABLE)
	valgrind --leak-check=full $(B_EXECUTABLE)

$(A_EXECUTABLE): $(A_SRC) $(A_HEADERS) $(B_EXECUTABLE)
	mkdir -p $(BUILD_DIR)
	$(CPP.COMPILER) $(A_SRC) $(CPP.FLAGS) -o $(A_EXECUTABLE) -I$(A_INCLUDE_DIR)

$(B_EXECUTABLE): $(B_SRC) $(B_HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CPP.COMPILER) $(B_SRC) $(CPP.FLAGS) -o $(B_EXECUTABLE) -I$(B_INCLUDE_DIR)

clean:
	rm -r build/
