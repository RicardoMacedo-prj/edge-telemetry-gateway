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
)

# Link Mosquitto library to gateway
target_link_libraries(gateway PRIVATE ${MOSQUITTO_LIB})