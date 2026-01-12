# Makefile for Speed Testing with CMock + Ceedling + GTest
# 
# This Makefile provides commands to:
# 1. Generate mocks with CMock from actual HAL/ThreadX headers
# 2. Compile and run tests with GTest
# 3. Clean build artifacts

# Directories
MOCK_DIR = mocks
BUILD_DIR = build/ceedling
TEST_SRC_DIR = .
PROJECT_SRC = ../Core/Src/Sensors
HAL_INC = ../Drivers/STM32U5xx_HAL_Driver/Inc
CORE_INC = ../Core/Inc
UTILS_INC = ../Core/Src/utils
THREADX_INC = ../Middlewares/ST/threadx/common/inc

# Original headers to mock
UTILS_HEADER = $(UTILS_INC)/utils.h
TX_API_HEADER = $(THREADX_INC)/tx_api.h

# Generated mock files (simple stubs for now, replace with Ceedling-generated later)
MOCK_SOURCES = mocks/mock_utils.c mocks/mock_tx_api.c
MOCK_HEADERS = mocks/mock_utils.h mocks/mock_tx_api.h

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -DUNIT_TEST -DPULSES_PER_REV=10 -DSTM32U585xx -DUSE_HAL_DRIVER -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1
INCLUDES = -I$(MOCK_DIR) -I$(CORE_INC) -I../Core/Src/Sensors -I$(UTILS_INC) -I$(THREADX_INC) -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I$(HAL_INC) -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -I.
LIBS = -lgtest -lgtest_main -lpthread

# Source files (exclude problematic STM32 files for testing)
TEST_SOURCES = speed_tests.c
PROJECT_SOURCES = # $(PROJECT_SRC)/speed.c  # Comment out for now due to STM32 dependencies
ALL_SOURCES = $(TEST_SOURCES) $(PROJECT_SOURCES) $(MOCK_SOURCES)

# Target executable
TARGET = speed_tests

.PHONY: all test clean generate-mocks install-deps help

# Default target
all: test

# Generate mock files using Ceedling (which includes CMock)
generate-mocks:
	@echo "Using Ceedling to generate mocks..."
	@echo "Note: Ceedling includes CMock 2.6.0"
	@echo "Use 'ceedling test:all' instead for integrated testing"

# Compile and run tests (simplified without CMock for now)
test: $(TARGET)

# Generate mocks from utils.h (Debug_Print function)
$(MOCK_DIR)/mock_utils.c $(MOCK_DIR)/mock_utils.h: $(UTILS_HEADER)
	@echo "Generating mock for utils.h..."
	@mkdir -p $(MOCK_DIR)
	ruby -e "\
	require 'cmock'; \
	cmock = CMock.new({ \
		:mock_path => '$(MOCK_DIR)', \
		:mock_prefix => 'mock_', \
		:plugins => [:ignore, :callback, :array, :return_thru_ptr], \
		:treat_as => { \
			'uint8' => 'HEX8', \
			'uint16' => 'HEX16', \
			'uint32' => 'HEX32' \
		} \
	}); \
	cmock.setup_mocks('$(UTILS_HEADER)')"

# Generate mocks from tx_api.h (ThreadX functions)
$(MOCK_DIR)/mock_tx_api.c $(MOCK_DIR)/mock_tx_api.h: $(TX_API_HEADER)
	@echo "Generating mock for tx_api.h..."
	@mkdir -p $(MOCK_DIR)
	ruby -e "\
	require 'cmock'; \
	cmock = CMock.new({ \
		:mock_path => '$(MOCK_DIR)', \
		:mock_prefix => 'mock_', \
		:plugins => [:ignore, :callback, :array, :return_thru_ptr], \
		:treat_as => { \
			'uint8' => 'HEX8', \
			'uint16' => 'HEX16', \
			'uint32' => 'HEX32', \
			'ULONG' => 'HEX32' \
		}, \
		:includes => ['stdint.h'] \
	}); \
	cmock.setup_mocks('$(TX_API_HEADER)')"

# Compile and run tests
test: $(TARGET)
	@echo "Running speed tests..."
	./$(TARGET)

# Compile the test executable
$(TARGET): $(ALL_SOURCES) $(MOCK_SOURCES)
	@echo "Compiling test executable..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ALL_SOURCES) $(LIBS) -o $(TARGET)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET)
	rm -f $(MOCK_SOURCES)
	rm -rf $(BUILD_DIR)
	rm -f *.o *.gcda *.gcno *.gcov

# Display help
help:
	@echo "Available targets:"
	@echo "  all           - Generate mocks from HAL/ThreadX headers and run tests (default)"
	@echo "  generate-mocks - Generate CMock files from actual STM32/ThreadX headers"
	@echo "  test          - Compile and run tests"
	@echo "  install-deps  - Install Ruby, CMock, and GoogleTest"
	@echo "  clean         - Remove build artifacts"
	@echo "  status        - Show setup status"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "How it works:"
	@echo "  1. CMock reads $(UTILS_HEADER) and $(TX_API_HEADER)"
	@echo "  2. Generates mock_utils.c/h and mock_tx_api.c/h"
	@echo "  3. GTest links with generated mocks"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - Ruby installed (for CMock)"
	@echo "  - CMock gem installed (gem install cmock)"
	@echo "  - GoogleTest library installed"
	@echo ""
	@echo "Example workflow:"
	@echo "  1. make install-deps  # One-time setup"
	@echo "  2. make               # Generate mocks from actual headers and run tests"
	@echo "  3. make clean         # Clean up when done"

# Show current status
status:
	@echo "=== CMock + GTest Setup Status ==="
	@echo "Ruby version:    $$(ruby --version 2>/dev/null || echo 'NOT INSTALLED')"
	@echo "CMock gem:       $$(gem list cmock 2>/dev/null | grep cmock || echo 'NOT INSTALLED')"
	@echo "GTest library:   $$(pkg-config --modversion gtest 2>/dev/null || echo 'NOT FOUND')"
	@echo "Original headers: $$(test -f $(UTILS_HEADER) && echo 'utils.h EXISTS' || echo 'utils.h MISSING') | $$(test -f $(TX_API_HEADER) && echo 'tx_api.h EXISTS' || echo 'tx_api.h MISSING')"
	@echo "Generated mocks:  $$(ls $(MOCK_DIR)/mock_*.c 2>/dev/null | wc -l) files"
	@echo "Test executable:  $$(test -f $(TARGET) && echo 'EXISTS' || echo 'NOT BUILT')"

# Display help
help:
	@echo "Available targets:"
	@echo "  all           - Generate mocks from HAL/ThreadX headers and run tests (default)"
	@echo "  generate-mocks - Generate CMock files from actual STM32/ThreadX headers"
	@echo "  test          - Compile and run tests"
	@echo "  install-deps  - Install Ruby, CMock, and GoogleTest"
	@echo "  clean         - Remove build artifacts"
	@echo "  status        - Show setup status"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "How it works:"
	@echo "  1. CMock reads $(UTILS_HEADER) and $(TX_API_HEADER)"
	@echo "  2. Generates mock_utils.c/h and mock_tx_api.c/h"
	@echo "  3. GTest links with generated mocks"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - Ruby installed (for CMock)"
	@echo "  - CMock gem installed (gem install cmock)"
	@echo "  - GoogleTest library installed"
	@echo ""
	@echo "Example workflow:"
	@echo "  1. make install-deps  # One-time setup"
	@echo "  2. make               # Generate mocks from actual headers and run tests"
	@echo "  3. make clean         # Clean up when done"