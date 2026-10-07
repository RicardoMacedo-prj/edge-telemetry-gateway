#include "mqtt_client.hpp"
#include <iostream>
#include <string>
#include <mosquitto.h>
#include "storage.hpp"

/**
 * @brief Callback triggered when broker acknowledges connection.
 */
void on_connect(struct mosquitto * /*mosq*/, void *userdata, int rc)
{
    // rc == 0 indicates successful MQTT connection acceptance
    if (rc == 0)
    {
        bool *is_connected = static_cast<bool *>(userdata);
        *is_connected = true;
        std::cout << "[Gateway] Connected to the MQTT broker." << std::endl;
    }
    else
    {
        std::cerr << "[Gateway] Failed to connect to the MQTT broker." << std::endl;
    }
}

/**
 * @brief Callback triggered when broker connection drops.
 */
void on_disconnect(struct mosquitto * /*mosq*/, void *userdata, int /*rc*/)
{
    bool *is_connected = static_cast<bool *>(userdata);
    *is_connected = false;
    std::cout << "[Gateway] Disconnected from the MQTT broker." << std::endl;
}

/**
 * @brief Publishes buffered telemetry records to the MQTT broker in batches.
 *
 * Reads up to max_records from telemetry.csv and transmits them in a single MQTT payload
 * with QoS 1. Upon successful publication, removes the transmitted records from the file
 * to prevent duplicate sends and free storage space.
 *
 * @param mosq Active Mosquitto client instance.
 * @param max_records Maximum number of records to retrieve in the batch.
 */
void send_telemetry(struct mosquitto *mosq, int max_records)
{
    auto [data, counter] = get_data_to_publish(max_records);

    if (data.empty())
    {
        return;
    }

    int rc = mosquitto_publish(mosq, nullptr, "car/telemetry", data.length(), data.c_str(), 1, false);

    if (rc != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Error publishing telemetry: " << mosquitto_strerror(rc) << std::endl;
    }
    else
    {
        remove_sent_data(counter);
    }
}