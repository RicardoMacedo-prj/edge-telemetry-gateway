cmake_minimum_required(VERSION 3.16)
project(EdgeTelemetryGateway VERSION 1.0.0 LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compiler warnings
add_compile_options(-Wall -Wextra)

# Find Mosquitto library
find_library(MOSQUITTO_LIB mosquitto REQUIRED)

# Vehicle simulation executable
add_executable(car car.cpp)

# Telemetry gateway executable
add_executable(gateway
    main.cpp
    storage.cpp
    decoder.cpp
    mqtt_client.cpp
    gatewayConfig.cpp
)

# Link Mosquitto library to gateway
target_link_libraries(gateway PRIVATE ${MOSQUITTO_LIB})

# Enable testing support
enable_testing()

# CAN decoder test executable
add_executable(test_can tests/test_can.cpp decoder.cpp)
target_include_directories(test_can PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestCan COMMAND test_can)

# Storage test executable
add_executable(test_storage tests/test_storage.cpp storage.cpp)
target_include_directories(test_storage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestStorage COMMAND test_storage)

# Gateway config test executable
add_executable(test_config tests/test_config.cpp gatewayConfig.cpp)
target_include_directories(test_config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME TestConfig COMMAND test_config)