cmake_minimum_required(VERSION 3.6)
project(alg_eng_matrix)

message(${CMAKE_CXX_COMPILER_ID})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("Adding -std=c++14 and -O3 for GNU C++ Compiler.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include -std=c++14 -fopenmp -O3") # -O2 -O3
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Adding /D_CRT_SECURE_NO_WARNINGS= /O2 /Ot /Og /Oi /Oy /Ox /Ob2 for MSVC.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS= /O2 /Ot /Oi /Oy /Ox /Ob2")
else ()
    message("Neither GNU nor MSVC: No additional flags added to the compiler.")
endif ()

set(outdir ${CMAKE_CURRENT_SOURCE_DIR}/output)
add_custom_target(build-time-make-dir ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir})

file(GLOB SOURCE_FILES
        "src/*.cpp"
        "src/*.hpp")

file(GLOB TEST_FILES
        "test/test.cpp"
        "test/lib/catch.hpp"
        "test/tests/*.cpp")

file(GLOB PCM
        "../pcm/*.o")

ADD_LIBRARY(papi STATIC IMPORTED)
SET_TARGET_PROPERTIES(papi PROPERTIES IMPORTED_LOCATION /usr/local/lib/libpapi.a)

message(${SOURCE_FILES})
add_library(CORE ${SOURCE_FILES} ${PCM})

add_executable(main ${SOURCE_FILES})
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${outdir})
target_link_libraries(main papi)

add_executable(testing ${TEST_FILES})
set_target_properties(testing PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${outdir})
target_link_libraries(testing CORE)

add_executable(benchmark "benchmark/benchmark.cpp")
set_target_properties(benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${outdir})
target_link_libraries(benchmark CORE papi)

add_executable(benchmark2 "benchmark/benchmark2.cpp")
set_target_properties(benchmark2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${outdir})
target_link_libraries(benchmark2 CORE papi)

add_executable(datagen "datagen/datagen.cpp")
set_target_properties(datagen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${outdir})
target_link_libraries(datagen CORE)
