#include <iostream>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <atomic>
#include <mosquitto.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include "car.hpp"

std::atomic<bool> running{true};

// Maximum number of telemetry records retained in local storage
const int MAX_RECORDS = 50;

/**
 * @brief Counts the total number of lines/records currently stored in telemetry.csv.
 * @return Line count.
 */
int count_csv_records()
{
    std::ifstream file("telemetry.csv");
    if (!file.is_open())
    {
        return 0;
    }

    int count = 0;
    std::string line;
    while (std::getline(file, line))
    {
        ++count;
    }
    file.close();
    return count;
}

/**
 * @brief Appends a telemetry record to telemetry.csv.
 * Enforces a strict FIFO retention policy by evicting the oldest record when full.
 */
bool save_telemetry_csv(int id, long long timestamp, double rpm, double speed, double coolantTemp)
{
    // FIFO eviction: if buffer is at capacity, discard the oldest line
    if (count_csv_records() >= MAX_RECORDS)
    {
        std::cerr << "Maximum record limit reached. Evicting oldest record." << std::endl;
        std::ifstream file("telemetry.csv");

        std::string first_line;
        std::getline(file, first_line); // Read and discard oldest record

        std::string line;
        std::string remaining_data;
        while (std::getline(file, line))
        {
            remaining_data += line + "\n";
        }
        file.close();

        // Overwrite file with remaining data (oldest line removed)
        std::ofstream file_out("telemetry.csv", std::ios::trunc);
        file_out << remaining_data;
        file_out.close();
    }

    // Append the new telemetry record to the end of the file
    std::ofstream file("telemetry.csv", std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Failed to open telemetry.csv!" << std::endl;
        return false;
    }

    file << id << "," << timestamp << "," << rpm << "," << speed << "," << coolantTemp << std::endl;
    file.close();
    return true;
}

/**
 * @brief Reads and displays all stored records from telemetry.csv.
 */
void read_telemetry_csv()
{
    std::ifstream file("telemetry.csv");
    if (!file.is_open())
    {
        std::cerr << "Failed to open telemetry.csv!" << std::endl;
        return;
    }

    std::string line;
    std::cout << "Telemetry Data from CSV:" << std::endl;

    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
    }
    file.close();
}

/**
 * @brief Scans telemetry.csv to determine the highest existing record ID.
 * Used during startup so new records can continue auto-incrementing sequentially.
 * @return Highest record ID found, or -1 if the file is empty or missing.
 */
long long get_max_id()
{
    std::ifstream file("telemetry.csv");
    if (!file.is_open())
    {
        std::cerr << "Failed to open telemetry.csv!" << std::endl;
        return -1;
    }

    long long max_id = -1;
    std::string line;
    std::cout << "Telemetry Data from CSV:" << std::endl;

    while (std::getline(file, line))
    {
        std::cout << line << std::endl;
        max_id = std::max(max_id, std::stoll(line.substr(0, line.find(','))));
    }
    file.close();
    return max_id;
}

/**
 * @brief Clears all records from telemetry.csv by truncating the file to zero bytes.
 */
void clear_telemetry_csv()
{
    std::ofstream file("telemetry.csv", std::ios::trunc);
    if (!file.is_open())
    {
        std::cerr << "Failed to open telemetry.csv!" << std::endl;
        return;
    }
    file.close();
    std::cout << "Telemetry CSV cleared." << std::endl;
}

// Signal handler for graceful shutdown on Ctrl+C
void handle_sigint(int /*signal*/)
{
    std::cout << "\n[Shutdown] Stopping cleanly..." << std::endl;
    running = false;
}

/**
 * @brief Publishes buffered telemetry records to the MQTT broker in batches.
 * 
 * Reads up to 50 records from telemetry.csv and transmits them in a single MQTT payload
 * with QoS 1. Upon successful publication, removes the transmitted records from the file
 * to prevent duplicate sends and free storage space.
 * 
 * @param mosq Active Mosquitto client instance.
 */
void send_telemetry_csv(struct mosquitto *mosq)
{
    std::ifstream file("telemetry.csv");

    std::string data_to_publish;
    std::string line;
    int counter = 0;
    while (std::getline(file, line) && counter < 50)
    {
        data_to_publish += line + "\n";
        counter++;
    }

    file.close();

    if (counter == 0)
    {
        return;
    }

    int rc = mosquitto_publish(mosq, nullptr, "car/telemetry", data_to_publish.length(), data_to_publish.c_str(), 1, false);

    if (rc != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Error publishing telemetry: " << mosquitto_strerror(rc) << std::endl;
    }
    else
    {
        std::ifstream file("telemetry.csv");

        std::string data_to_rewrite;

        for (int i = 0; i < counter && std::getline(file, line); i++)
        {
        }

        while (std::getline(file, line))
        {
            data_to_rewrite += line + "\n";
        }

        file.close();

        std::ofstream file_out("telemetry.csv", std::ios::trunc);
        file_out << data_to_rewrite;
        file_out.close();
    }
}

/**
 * @brief Decodes an 8-byte OBD-II CAN frame and prints reconstructed metric.
 */
void decode_can(CanFrame canFrame)
{
    // Inspect byte 1 to determine the PID sensor type
    if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::rpm))
    {
        uint16_t rpmData = (static_cast<uint16_t>(canFrame.data[2]) << 8) | static_cast<uint16_t>(canFrame.data[3]);
        double rpm = static_cast<double>(rpmData) / 4.0;
        std::cout << "Decoded RPM: " << rpm << std::endl;
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::speed))
    {
        double speed = static_cast<double>(canFrame.data[2]);
        std::cout << "Decoded Speed: " << speed << " km/h" << std::endl;
    }
    else if (canFrame.data[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
    {
        double coolantTemp = static_cast<double>(canFrame.data[2]) - 40.0;
        std::cout << "Decoded Coolant Temperature: " << coolantTemp << std::endl;
    }
}

int main()
{
    // Register the signal handler for Ctrl+C
    std::signal(SIGINT, handle_sigint);

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a UDP socket for sending CAN frames
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Failed to create socket!" << std::endl;
        return 1;
    }

    // UDP socket address configuration for localhost:4000
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    socklen_t addr_len = sizeof(addr);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Failed to bind socket!" << std::endl;
        return 1;
    }

    // Configure socket receive timeout to prevent blocking indefinitely when no packets arrive.
    // This allows the loop to periodically check the 'running' flag on Ctrl+C.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500 ms timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Create a new Mosquitto client instance
    struct mosquitto *mosq = mosquitto_new("can-publisher", true, NULL);
    if (!mosq)
    {
        std::cerr << "Failed to create Mosquitto instance." << std::endl;
        return 1;
    }

    bool is_connected = false;

    // Connect to local Mosquitto MQTT broker
    int rc = mosquitto_connect(mosq, "127.0.0.1", 1883, 60);

    if (rc == MOSQ_ERR_SUCCESS)
    {
        is_connected = true;
        mosquitto_loop_start(mosq); // Start background networking thread
        std::cout << "[Gateway] Connected to the MQTT broker." << std::endl;
    }
    else
    {
        is_connected = false;
        std::cerr << "[Gateway] Failed to connect to the MQTT broker." << std::endl;
    }

    // Buffer to receive 8-byte CAN frames over UDP
    uint8_t receiveBuffer[8];

    // Determine starting ID once at startup to avoid repeated disk I/O in the main loop
    long long max_id = get_max_id();
    if (max_id == -1)
    {
        std::cout << "[Decoder] No records found in telemetry.csv" << std::endl;
        max_id = 0;
    }

    // Continuously listen for incoming CAN frames
    while (running)
    {

        ssize_t bytes_received = recvfrom(sock, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *)&addr, &addr_len);

        if (bytes_received < 0)
        {
            continue;
        }

        if (receiveBuffer[1] == static_cast<uint8_t>(PID_standart::rpm))
        {
            save_telemetry_csv(
                max_id += 1,
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
                (static_cast<uint16_t>(receiveBuffer[2]) << 8 | static_cast<uint16_t>(receiveBuffer[3])) / 4.0,
                0.0,
                0.0);
        }
        else if (receiveBuffer[1] == static_cast<uint8_t>(PID_standart::speed))
        {
            save_telemetry_csv(
                max_id += 1,
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
                0.0,
                static_cast<double>(receiveBuffer[2]),
                0.0);
        }
        else if (receiveBuffer[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
        {
            save_telemetry_csv(
                max_id += 1,
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(),
                0.0,
                0.0,
                static_cast<double>(receiveBuffer[2]) - 40);
        }

        // Store-and-forward: flush buffered telemetry records to the cloud broker if connected
        if (is_connected)
        {
            send_telemetry_csv(mosq);
        }
    }

    // Close the UDP socket
    close(sock);

    // Stop the Mosquitto background thread, disconnect, and clean up resources
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}