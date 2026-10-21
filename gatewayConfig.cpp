#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include "gatewayConfig.hpp"

/**
 * @brief Removes leading and trailing whitespace characters.
 *
 * Ensures the input string contains only numeric digits (0-9) before conversion.
 */
static std::string trim(const std::string &value)
{
    const auto start = value.find_first_not_of(" \t\r\n");

    // String contains only whitespace
    if (start == std::string::npos)
        return "";

    const auto end = value.find_last_not_of(" \t\r\n");

    return value.substr(start, end - start + 1);
}

/**
 * @brief Parses an unsigned decimal integer string into an unsigned long long.
 *
 * Ensures the input string contains only numeric digits (0-9) before conversion.
 */
static bool parseUnsigned(const std::string &value, unsigned long long &result)
{
    if (value.empty() ||
        value.find_first_not_of("0123456789") != std::string::npos)
    {
        return false;
    }

    try
    {
        result = std::stoull(value);
        return true;
    }
    catch (const std::exception &)
    {
        // Catches numeric overflow if value exceeds unsigned long long range
        return false;
    }
}

/**
 * @brief Extracts and validates a numeric configuration parameter from the map.
 */
static bool parseConfigNumber(
    const std::unordered_map<std::string, std::string> &values,
    const std::string &key,
    unsigned long long &result)
{
    if (!parseUnsigned(values.at(key), result))
    {
        std::cerr
            << "Config error: Invalid numeric value for "
            << key << ": '" << values.at(key) << "'\n";

        return false;
    }

    return true;
}

/**
 * @brief Validates a standard IPv4 dotted-decimal address (A.B.C.D).
 *
 * Checks if the string contains exactly 4 numeric octets separated by dots,
 * each between 0 and 255. Rejects empty octets and ambiguous leading zeroes.
 */
static bool isValidIpv4(const std::string &ip)
{
    size_t start = 0;
    int octetCount = 0;

    while (start <= ip.length())
    {
        size_t end = ip.find('.', start);
        
        // If no further dot is found, read until end of string
        if (end == std::string::npos)
        {
            end = ip.length();
        }

        std::string part = ip.substr(start, end - start);
        
        // Catch empty segments like "192..1.1" or trailing dot "192.168.1."
        if (part.empty())
        {
            return false;
        }

        // Validate octet is numeric and in 0..255 byte range
        unsigned long long value = 0;
        if (!parseUnsigned(part, value) || value > 255)
        {
            return false;
        }

        // Catch ambiguous leading zeros like "01" (single "0" is valid)
        if (part.size() > 1 && part[0] == '0')
        {
            return false;
        }

        octetCount++;

        // Reached end of string
        if (end == ip.length())
        {
            break;
        }

        start = end + 1;
    }

    // Must have exactly 4 octets
    return octetCount == 4;
}

bool loadConfig(const std::string &path, GatewayConfig &config)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "Config error: Cannot open file: " << path << '\n';
        return false;
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;

    // Read configuration line by line
    while (std::getline(file, line))
    {
        line = trim(line);

        // Skip blank lines and comments
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        // Require KEY=VALUE delimiter
        const auto eqPos = line.find('=');
        if (eqPos == std::string::npos)
        {
            std::cerr << "Config error: Invalid configuration entry: '" << line << "'\n";
            return false;
        }

        const std::string key = trim(line.substr(0, eqPos));
        const std::string value = trim(line.substr(eqPos + 1));

        if (key.empty() || value.empty())
        {
            std::cerr << "Config error: Empty key or value in configuration entry: '" << line << "'\n";
            return false;
        }

        // Reject duplicate keys to avoid ambiguous overwrite behavior
        if (!values.emplace(key, value).second)
        {
            std::cerr << "Config error: Duplicate key: " << key << '\n';
            return false;
        }
    }

    // Check for underlying I/O errors during reading
    if (file.bad())
    {
        std::cerr << "Config error: Failed to read file: " << path << '\n';
        return false;
    }

    // Verify all mandatory keys exist
    const char *required[] = {"SOCKET_ADDRESS", "SOCKET_PORT", "MOSQUITTO_BROKER_ADDRESS",
                              "MOSQUITTO_BROKER_PORT", "MAX_STORAGE_RECORDS", "MAX_CHUNK_SIZE"};

    for (const char *reqKey : required)
    {
        if (values.find(reqKey) == values.end())
        {
            std::cerr << "Config error: Missing required key: " << reqKey << '\n';
            return false;
        }
    }

    // Parse and validate numeric parameters
    unsigned long long socketPort;
    unsigned long long mosquittoBrokerPort;
    unsigned long long maxStorageRecords;
    unsigned long long maxChunkSize;

    if (!parseConfigNumber(values, "SOCKET_PORT", socketPort) ||
        !parseConfigNumber(values, "MOSQUITTO_BROKER_PORT", mosquittoBrokerPort) ||
        !parseConfigNumber(values, "MAX_STORAGE_RECORDS", maxStorageRecords) ||
        !parseConfigNumber(values, "MAX_CHUNK_SIZE", maxChunkSize))
    {
        return false;
    }

    // Enforce business boundaries on storage retention and batch chunk sizes
    if (maxStorageRecords > 100000)
    {
        std::cerr << "Config error: MAX_STORAGE_RECORDS cannot exceed 100000. Provided value: " << maxStorageRecords << '\n';
        return false;
    }

    if (maxChunkSize > 10000)
    {
        std::cerr << "Config error: MAX_CHUNK_SIZE cannot exceed 10000. Provided value: " << maxChunkSize << '\n';
        return false;
    }

    // Validate valid TCP/UDP port range (1..65535)
    if (socketPort < 1 || socketPort > 65535)
    {
        std::cerr << "Config error: SOCKET_PORT must be between 1 and 65535. "
                  << "Provided value: " << socketPort << '\n';
        return false;
    }
    if (mosquittoBrokerPort < 1 || mosquittoBrokerPort > 65535)
    {
        std::cerr << "Config error: MOSQUITTO_BROKER_PORT must be between 1 and 65535. "
                  << "Provided value: " << mosquittoBrokerPort << '\n';
        return false;
    }

    // Validate network addresses (valid IPv4 or local loopback name)
    std::string socketAddress = values.at("SOCKET_ADDRESS");
    std::string MosquittoBrokerAddress = values.at("MOSQUITTO_BROKER_ADDRESS");

    if (!isValidIpv4(socketAddress) && socketAddress != "localhost")
    {
        std::cerr << "Config error: Invalid IPv4 address for SOCKET_ADDRESS. "
                  << "Provided value: " << socketAddress << "'\n";
        return false;
    }

    if (!isValidIpv4(MosquittoBrokerAddress) && MosquittoBrokerAddress != "localhost")
    {
        std::cerr << "Config error: Invalid IPv4 address for MOSQUITTO_BROKER_ADDRESS. "
                  << "Provided value: " << MosquittoBrokerAddress << "'\n";
        return false;
    }

    // Apply parsed values to config struct only after all validations pass
    config.SOCKET_ADDRESS = values.at("SOCKET_ADDRESS");
    config.SOCKET_PORT = static_cast<std::uint16_t>(socketPort);
    config.MOSQUITTO_BROKER_ADDRESS = values.at("MOSQUITTO_BROKER_ADDRESS");
    config.MOSQUITTO_BROKER_PORT = static_cast<std::uint16_t>(mosquittoBrokerPort);
    config.MAX_STORAGE_RECORDS = static_cast<std::uint32_t>(maxStorageRecords);
    config.MAX_CHUNK_SIZE = static_cast<std::uint32_t>(maxChunkSize);

    return true;
}
