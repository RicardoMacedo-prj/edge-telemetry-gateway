#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include "gatewayConfig.hpp"

// Creates a temporary config file with specified content
static void create_temp_file(const std::string &path, const std::string &content)
{
    std::ofstream file(path);
    file << content;
    file.close();
}

void test_valid_config()
{
    const std::string temp_path = "test_valid.conf";
    const std::string content =
        "# Telemetry Gateway Valid Config\n"
        "SOCKET_ADDRESS = 127.0.0.1\n"
        "SOCKET_PORT = 4000\n"
        "MOSQUITTO_BROKER_ADDRESS = 192.168.1.50\n"
        "MOSQUITTO_BROKER_PORT = 1883\n"
        "MAX_STORAGE_RECORDS = 5000\n"
        "MAX_CHUNK_SIZE = 500\n";

    create_temp_file(temp_path, content);

    GatewayConfig config;
    bool ok = loadConfig(temp_path, config);

    assert(ok == true);
    assert(config.SOCKET_ADDRESS == "127.0.0.1");
    assert(config.SOCKET_PORT == 4000);
    assert(config.MOSQUITTO_BROKER_ADDRESS == "192.168.1.50");
    assert(config.MOSQUITTO_BROKER_PORT == 1883);
    assert(config.MAX_STORAGE_RECORDS == 5000);
    assert(config.MAX_CHUNK_SIZE == 500);

    std::remove(temp_path.c_str());
}

void test_nonexistent_file()
{
    GatewayConfig config;
    bool ok = loadConfig("nonexistent_file_path.conf", config);

    assert(ok == false);
}

void test_invalid_port()
{
    const std::string temp_path = "test_invalid_port.conf";
    // Port 70000 exceeds valid 16-bit TCP/UDP range (1..65535)
    const std::string content =
        "SOCKET_ADDRESS = 127.0.0.1\n"
        "SOCKET_PORT = 70000\n"
        "MOSQUITTO_BROKER_ADDRESS = 127.0.0.1\n"
        "MOSQUITTO_BROKER_PORT = 1883\n"
        "MAX_STORAGE_RECORDS = 1000\n"
        "MAX_CHUNK_SIZE = 100\n";

    create_temp_file(temp_path, content);

    GatewayConfig config;
    bool ok = loadConfig(temp_path, config);

    assert(ok == false);

    std::remove(temp_path.c_str());
}

void test_invalid_ip()
{
    const std::string temp_path = "test_invalid_ip.conf";
    // 300 is outside 0..255 byte boundary
    const std::string content =
        "SOCKET_ADDRESS = 127.0.0.300\n"
        "SOCKET_PORT = 4000\n"
        "MOSQUITTO_BROKER_ADDRESS = 127.0.0.1\n"
        "MOSQUITTO_BROKER_PORT = 1883\n"
        "MAX_STORAGE_RECORDS = 1000\n"
        "MAX_CHUNK_SIZE = 100\n";

    create_temp_file(temp_path, content);

    GatewayConfig config;
    bool ok = loadConfig(temp_path, config);

    assert(ok == false);

    std::remove(temp_path.c_str());
}

void test_missing_key()
{
    const std::string temp_path = "test_missing_key.conf";
    // Missing MAX_CHUNK_SIZE
    const std::string content =
        "SOCKET_ADDRESS = 127.0.0.1\n"
        "SOCKET_PORT = 4000\n"
        "MOSQUITTO_BROKER_ADDRESS = 127.0.0.1\n"
        "MOSQUITTO_BROKER_PORT = 1883\n"
        "MAX_STORAGE_RECORDS = 1000\n";

    create_temp_file(temp_path, content);

    GatewayConfig config;
    bool ok = loadConfig(temp_path, config);

    assert(ok == false);

    std::remove(temp_path.c_str());
}

void test_duplicate_key()
{
    const std::string temp_path = "test_duplicate_key.conf";
    // Duplicate SOCKET_PORT entry
    const std::string content =
        "SOCKET_ADDRESS = 127.0.0.1\n"
        "SOCKET_PORT = 4000\n"
        "SOCKET_PORT = 5000\n"
        "MOSQUITTO_BROKER_ADDRESS = 127.0.0.1\n"
        "MOSQUITTO_BROKER_PORT = 1883\n"
        "MAX_STORAGE_RECORDS = 1000\n"
        "MAX_CHUNK_SIZE = 100\n";

    create_temp_file(temp_path, content);

    GatewayConfig config;
    bool ok = loadConfig(temp_path, config);

    assert(ok == false);

    std::remove(temp_path.c_str());
}

int main()
{
    test_valid_config();
    test_nonexistent_file();
    test_invalid_port();
    test_invalid_ip();
    test_missing_key();
    test_duplicate_key();

    return 0;
}
