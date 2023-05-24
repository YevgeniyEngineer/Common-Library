cmake_minimum_required(VERSION 3.18)

project(common_library)

add_library(${PROJECT_NAME}
    INTERFACE
    concurrency/ts_queue.hpp
    concurrency/spsc_queue.hpp
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(example_spsc_queue examples/spsc_queue.cpp)
target_link_libraries(example_spsc_queue PRIVATE common_library)

add_executable(example_ts_queue examples/ts_queue.cpp)
target_link_libraries(example_ts_queue PRIVATE common_library)