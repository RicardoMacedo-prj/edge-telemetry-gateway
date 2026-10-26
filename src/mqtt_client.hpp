#pragma once

#include <mosquitto.h>
#include <string>

/**
 * @brief Mosquitto connection callback.
 * 
 * Invoked by libmosquitto when the MQTT broker responds to the connection request.
 * Sets the user-provided is_connected boolean flag to true on success (rc == 0).
 * 
 * @param mosq Active Mosquitto client instance.
 * @param userdata Pointer to the user-supplied application state (bool* is_connected).
 * @param rc Connect return code (0 indicates connection accepted).
 */
void on_connect(struct mosquitto *mosq, void *userdata, int rc);

/**
 * @brief Mosquitto disconnect callback.
 * 
 * Invoked by libmosquitto whenever connection to the MQTT broker is lost.
 * Resets the user-provided is_connected boolean flag to false.
 * 
 * @param mosq Active Mosquitto client instance.
 * @param userdata Pointer to the user-supplied application state (bool* is_connected).
 * @param rc Disconnect reason code.
 */
void on_disconnect(struct mosquitto *mosq, void *userdata, int rc);

/**
 * @brief Reads a pending batch from local storage and publishes it via MQTT.
 * 
 * Retrieves up to max_chunk_size records via get_data_to_publish() and publishes them to the
 * "vehicle/telemetry" topic with QoS 1. On successful transmission, calls remove_sent_data()
 * to delete the transmitted batch from local disk.
 * 
 * @param mosq Active Mosquitto client instance.
 * @param max_chunk_size Maximum number of records to retrieve in the batch (defaults to 100).
 */
void send_telemetry(struct mosquitto *mosq, int max_chunk_size = 100);

/**
 * @brief Publishes an alert payload to the MQTT broker.
 * 
 * Formats the alert as a JSON message and sends it to the "vehicle/alerts/critical" topic.
 * 
 * @param mosq Active Mosquitto client instance.
 * @param data Sensor value that caused the alert.
 * @param alert_type Alert identifier (e.g., "HIGH_RPM").
 * @param threshold Limit value that was exceeded.
 * @param timestamp Timestamp in milliseconds when the metric was generated.
 */
void send_alert_telemetry(struct mosquitto *mosq, double data, std::string alert_type, double threshold, long long timestamp);