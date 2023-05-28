cmake_minimum_required(VERSION 3.18)

project(common_library)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(${PROJECT_NAME}
    INTERFACE
    common_library/concurrency/thread_safe_queue.hpp
    common_library/concurrency/single_producer_single_consumer_queue.hpp
    common_library/concurrency/lock_free_queue.hpp
    common_library/concurrency/thread_safe_logger.hpp
    common_library/concurrency/bounded_shared_queue.hpp
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    -latomic
)

add_executable(example_single_producer_single_consumer_queue examples/single_producer_single_consumer_queue.cpp)
target_link_libraries(example_single_producer_single_consumer_queue PRIVATE common_library)

add_executable(example_thread_safe_queue examples/thread_safe_queue.cpp)
target_link_libraries(example_thread_safe_queue PRIVATE common_library)

add_executable(example_lock_free_queue examples/lock_free_queue.cpp)
target_link_libraries(example_lock_free_queue PRIVATE common_library)

add_executable(example_bounded_shared_queue examples/bounded_shared_queue.cpp)
target_link_libraries(example_bounded_shared_queue PRIVATE common_library)