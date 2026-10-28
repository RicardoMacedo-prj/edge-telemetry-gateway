cmake_minimum_required(VERSION 3.16)
project(EdgeTelemetryGateway VERSION 1.0.0 LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compiler warnings
add_compile_options(-Wall -Wextra)

# Include third-party headers
include_directories(src external)

# Find Mosquitto library
find_library(MOSQUITTO_LIB mosquitto REQUIRED)

# Vehicle simulation executable
add_executable(car src/vehicle.cpp)

# Telemetry gateway executable
add_executable(gateway
    src/main.cpp
    src/storage.cpp
    src/decoder.cpp
    src/mqtt_client.cpp
    src/gatewayConfig.cpp
    src/rules.cpp
    src/thread_safe_queue.cpp
    src/producer.cpp
    src/consumer.cpp
)

# Link Mosquitto library to gateway
target_link_libraries(gateway PRIVATE ${MOSQUITTO_LIB})

# Enable testing support
enable_testing()

# CAN decoder test executable
add_executable(test_can tests/test_can.cpp src/decoder.cpp)
target_include_directories(test_can PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestCan COMMAND test_can)

# Storage test executable
add_executable(test_storage tests/test_storage.cpp src/storage.cpp)
target_include_directories(test_storage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestStorage COMMAND test_storage)

# Gateway config test executable
add_executable(test_config tests/test_config.cpp src/gatewayConfig.cpp)
target_include_directories(test_config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestConfig COMMAND test_config)

# Rules test executable
add_executable(test_rules tests/test_rules.cpp src/rules.cpp)
target_include_directories(test_rules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestRules COMMAND test_rules)