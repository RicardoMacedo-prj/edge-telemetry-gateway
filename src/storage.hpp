#pragma once

#include <string>
#include <utility>
#include <vector>
#include "gatewayConfig.hpp"

/**
 * @brief Represents a single decoded telemetry record stored in memory.
 */
struct TelemetryRecord 
{
    int id;
    long long timestamp;
    double rpm;
    double speed;
    double coolantTemp;
};

/**
 * @brief Holds a batch of telemetry records and the count of items to publish.
 */
struct DataToPublish
{
    std::vector<TelemetryRecord> data;
    int counter = 0;
};

/**
 * @brief Initializes the SQLite database, creating the telemetry table and enabling WAL mode.
 * @param file Database file path (defaults to "telemetry.db").
 */
void init_storage(const std::string& file = "telemetry.db");

/**
 * @brief Closes the SQLite database connection if open.
 */
void close_storage();

/**
 * @brief Counts the total number of telemetry records stored in the database.
 * @return Number of records stored in the database, or -1 on error.
 */
int count_csv_records();

/**
 * @brief Inserts a telemetry record into the database, enforcing a FIFO limit.
 * 
 * If the current record count exceeds max_storage_records, the oldest record
 * is evicted using an atomic SQL DELETE before inserting the new record.
 * 
 * @param id Unique record identifier.
 * @param timestamp Timestamp in milliseconds since Unix epoch.
 * @param rpm Engine RPM metric.
 * @param speed Vehicle speed metric (km/h).
 * @param coolantTemp Engine coolant temperature (Celsius).
 * @param max_storage_records Maximum capacity of the local buffer before FIFO eviction.
 * @return true if write succeeded, false otherwise.
 */
bool save_telemetry_csv(int id, long long timestamp, double rpm, double speed, double coolantTemp, int max_storage_records);

/**
 * @brief Reads and prints all stored telemetry records to standard output.
 */
void read_telemetry_csv();

/**
 * @brief Retrieves a batch of up to max_chunk_size telemetry records for transmission.
 * 
 * Queries records ordered by ID ascending to prepare an MQTT batch payload.
 * 
 * @param max_chunk_size Maximum number of records to retrieve in the batch (defaults to 100).
 * @return DataToPublish struct containing the vector of records and count.
 */
DataToPublish get_data_to_publish(int max_chunk_size = 100);

/**
 * @brief Deletes the specified number of sent records from the database.
 * 
 * @param counter Number of transmitted records to remove.
 */
void remove_sent_data(int counter);

/**
 * @brief Queries the highest existing record ID in the database.
 * 
 * Used during gateway startup so sequential ID auto-incrementing resumes correctly.
 * 
 * @return Highest record ID found, or -1 if the table is empty or on error.
 */
long long get_max_id();

/**
 * @brief Deletes all records from the telemetry database table.
 */
void clear_telemetry_csv();

