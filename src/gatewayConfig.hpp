#pragma once

#include <cstdint>
#include <string>

/**
 * @brief Holds runtime configuration settings for the telemetry gateway.
 *
 * Provides initial default values for the gateway configuration structure.
 */
struct GatewayConfig 
{
    std::string SOCKET_ADDRESS = "127.0.0.1";
    int SOCKET_PORT = 4000;
    std::string MOSQUITTO_BROKER_ADDRESS = "127.0.0.1";
    int MOSQUITTO_BROKER_PORT = 1883;
    int MAX_STORAGE_RECORDS = 10000;
    int MAX_CHUNK_SIZE = 1000;
};

/**
 * @brief Loads and parses gateway configuration from a key-value text file.
 *
 * Reads lines formatted as KEY = VALUE, ignoring empty lines and comments (#).
 * Performs strict validation on data types, network ranges, and required fields.
 * If validation fails, the configuration is rejected and false is returned.
 *
 * @param path Filesystem path to the configuration file.
 * @param config Output GatewayConfig structure populated on success.
 * @return true if file was loaded and all validations passed, false otherwise.
 */
bool loadConfig(const std::string& path, GatewayConfig& config);

