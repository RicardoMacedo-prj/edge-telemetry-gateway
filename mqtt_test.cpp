#include <iostream>
#include <string>
#include <mosquitto.h>
#include <thread>
#include <chrono>

int main() {
    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto instance
    struct mosquitto *mosq = mosquitto_new("test-publisher", true, NULL);
    if (!mosq) {
        std::cerr << "Failed to create Mosquitto instance." << std::endl;
        return 1;
    }

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, "127.0.0.1", 1883, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to the broker." << std::endl;
        mosquitto_destroy(mosq);
        return 1;
    }

    std::cout << "Connected to the broker. Publishing telemetry data..." << std::endl;

    // Start the network loop in a separate thread
    mosquitto_loop_start(mosq);

    // Publish telemetry data
    std::string msg1 = "{\"rpm\": 1500, \"speed\": 40, \"temp\": 75}";
    std::string msg2 = "{\"rpm\": 2500, \"speed\": 75, \"temp\": 85}";
    std::string msg3 = "{\"rpm\": 3800, \"speed\": 110, \"temp\": 90}";

    mosquitto_publish(mosq, nullptr, "car/telemetry", msg1.length(), msg1.c_str(), 1, false);
    mosquitto_publish(mosq, nullptr, "car/telemetry", msg2.length(), msg2.c_str(), 1, false);
    mosquitto_publish(mosq, nullptr, "car/telemetry", msg3.length(), msg3.c_str(), 1, false);

    std::cout << "Telemetry data published." << std::endl;

    // Wait for a moment to ensure messages are sent
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Stop the network loop
    mosquitto_loop_stop(mosq, false);

    // Disconnect and clean up resources
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}