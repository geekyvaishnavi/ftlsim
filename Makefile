# Fallback build for environments without CMake. CMakeLists.txt is the
# canonical build; this produces the same two binaries under build/.
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude

BUILD    := build
CORE_SRC := src/nand/page.cpp src/nand/block.cpp src/nand/nand.cpp \
            src/ftl/mapping.cpp src/ftl/ftl.cpp src/ftl/gc.cpp src/ftl/wearlevel.cpp \
            src/metrics/metrics.cpp src/workload/generator.cpp src/workload/trace.cpp
CORE_OBJ := $(CORE_SRC:%.cpp=$(BUILD)/%.o)

.PHONY: all test clean
all: $(BUILD)/ftlsim $(BUILD)/ftlsim_tests

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/ftlsim: $(CORE_OBJ) $(BUILD)/src/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/ftlsim_tests: $(CORE_OBJ) $(BUILD)/tests/test_ftlsim.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(BUILD)/ftlsim_tests
	./$(BUILD)/ftlsim_tests

clean:
	rm -rf $(BUILD)
