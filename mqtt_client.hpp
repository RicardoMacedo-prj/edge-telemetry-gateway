#pragma once

#include <mosquitto.h>

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
 * Retrieves up to max_records via get_data_to_publish() and publishes them to the
 * "car/telemetry" topic with QoS 1. On successful transmission, calls remove_sent_data()
 * to delete the transmitted batch from local disk.
 * 
 * @param mosq Active Mosquitto client instance.
 * @param max_records Maximum number of records to retrieve in the batch (defaults to 50).
 */
void send_telemetry(struct mosquitto *mosq, int max_records = 50);