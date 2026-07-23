cmake_minimum_required(VERSION 3.16)
project(ftlsim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Simulation core, shared by the CLI and the tests.
add_library(ftlsim_core
  src/nand/page.cpp
  src/nand/block.cpp
  src/nand/nand.cpp
  src/ftl/mapping.cpp
  src/ftl/ftl.cpp
  src/ftl/gc.cpp
  src/ftl/wearlevel.cpp
  src/metrics/metrics.cpp
  src/workload/generator.cpp
  src/workload/trace.cpp
)
target_include_directories(ftlsim_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ftlsim src/main.cpp)
target_link_libraries(ftlsim PRIVATE ftlsim_core)

enable_testing()

add_executable(ftlsim_tests tests/test_ftlsim.cpp)
target_link_libraries(ftlsim_tests PRIVATE ftlsim_core)
add_test(NAME ftlsim_tests COMMAND ftlsim_tests)
