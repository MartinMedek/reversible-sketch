cmake_minimum_required(VERSION 3.3)
project(sketch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
# -fsanitize=address -fno-omit-frame-pointer

set(SOURCE_FILES2 src/SketchRange.h src/demo.cpp src/Sketch.h src/SketchTable.h src/demo.cpp src/misc_functions.h src/BIV.h src/Sketch.cpp src/SketchTable.cpp src/helpers.h)
add_executable(demo ${SOURCE_FILES2})



set(SOURCE_FILES src/catch.hpp src/tests.cpp src/SketchTable.h src/SketchTable.cpp
        src/BIV.h src/misc_functions.h src/helpers.h src/Sketch.h src/Sketch.cpp)
add_executable(tests ${SOURCE_FILES})

#include(cmake/clang-tidy.cmake)
