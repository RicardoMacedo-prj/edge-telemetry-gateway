#pragma once

#include <string>
#include <utility>

/**
 * @brief Counts the total number of telemetry records currently stored in telemetry.csv.
 * @return Number of lines/records in the file, or 0 if the file cannot be opened.
 */
int count_csv_records();

/**
 * @brief Appends a telemetry record to telemetry.csv enforcing a FIFO limit.
 * 
 * If the current record count exceeds max_records, the oldest record is evicted
 * before appending the new record.
 * 
 * @param id Unique record identifier.
 * @param timestamp Timestamp in milliseconds since Unix epoch.
 * @param rpm Engine RPM metric.
 * @param speed Vehicle speed metric (km/h).
 * @param coolantTemp Engine coolant temperature (Celsius).
 * @param max_records Maximum capacity of the local buffer before FIFO eviction.
 * @return true if write succeeded, false otherwise.
 */
bool save_telemetry_csv(int id, long long timestamp, double rpm, double speed, double coolantTemp, int max_records);

/**
 * @brief Reads and prints all telemetry records from telemetry.csv to standard output.
 */
void read_telemetry_csv();

/**
 * @brief Reads up to max_records pending telemetry records for transmission.
 * 
 * Does not modify the file; simply reads the oldest batch of records to prepare
 * for MQTT publishing.
 * 
 * @param max_records Maximum number of records to retrieve in the batch (defaults to 50).
 * @return A pair containing:
 *         - first: String containing concatenated lines to publish.
 *         - second: Count of lines included in the batch.
 */
std::pair<std::string, int> get_data_to_publish(int max_records = 50);

/**
 * @brief Removes the specified number of sent records from the top of telemetry.csv.
 * 
 * Skips the first 'counter' lines and rewrites the remainder back to telemetry.csv.
 * 
 * @param counter Number of oldest records to discard after successful transmission.
 */
void remove_sent_data(int counter);

/**
 * @brief Scans telemetry.csv to determine the highest existing record ID.
 * 
 * Used during gateway startup to resume sequential ID auto-incrementing in memory.
 * 
 * @return Highest record ID found, or -1 if the file is empty or missing.
 */
long long get_max_id();

/**
 * @brief Truncates telemetry.csv, removing all stored records.
 */
void clear_telemetry_csv();

