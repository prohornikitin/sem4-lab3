CPP.COMPILER ?= g++
BUILD ?= debug

DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR := $(DIR)/src
INCLUDE_DIR := $(SRC_DIR)
BUILD_DIR := $(DIR)/build/$(BUILD)

EXECUTABLE := $(BUILD_DIR)/exec

CPP.FLAGS := -pedantic -Wall -Wextra -std=gnu++20 -lreadline
ifeq ($(BUILD),release)
	CPP.FLAGS += -Ofast
else
	CPP.FLAGS += -g -O0 -D DEBUG
endif

SRC := $(shell find $(SRC_DIR) -name '*.cpp')
HEADERS := $(shell find $(INCLUDE_DIR) -name '*.h')

all: $(EXECUTABLE)

run: $(EXECUTABLE)
	exec $(EXECUTABLE)

run-vg: $(EXECUTABLE)
	valgrind --leak-check=full $(EXECUTABLE)

$(EXECUTABLE): $(SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	$(CPP.COMPILER) $(SRC) $(CPP.FLAGS) -o $(EXECUTABLE) -I$(INCLUDE_DIR)

clean:
	rm -r build/
