cmake_minimum_required(VERSION 3.20)
project(BlackHoleTracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_package(OpenMP)

add_executable(blackhole_tracer
        src/main.cpp
        src/Geodesic.cpp
        src/AccretionDisk.cpp
        src/Dither.cpp
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(blackhole_tracer PRIVATE OpenMP::OpenMP_CXX)
endif()

find_program(FFMPEG_EXECUTABLE ffmpeg)

if(FFMPEG_EXECUTABLE)
    add_custom_target(render
            COMMAND ${CMAKE_BINARY_DIR}/blackhole_tracer
            COMMAND ${FFMPEG_EXECUTABLE} -y -i ${CMAKE_BINARY_DIR}/blackhole.ppm ${CMAKE_BINARY_DIR}/blackhole.png
            COMMENT "Simulation des geodesiques et conversion en blackhole.png via ffmpeg"
            DEPENDS blackhole_tracer
    )
endif()

add_executable(test_ray benchmarks/test_ray.cpp src/Geodesic.cpp)
target_include_directories(test_ray PRIVATE include)