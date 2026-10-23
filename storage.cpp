#include "storage.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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
bool save_telemetry_csv(int id, long long timestamp, double rpm, double speed, double coolantTemp, int max_storage_records)
{
    // FIFO eviction: if buffer is at capacity, discard the oldest line
    if (count_csv_records() >= max_storage_records)
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
 * @brief Reads up to max_chunk_size stored records from telemetry.csv to prepare an MQTT batch.
 */
DataToPublish get_data_to_publish(int max_chunk_size)
{
    std::ifstream file("telemetry.csv");
    
    std::string line;
    DataToPublish result;

    // Read up to max_chunk_size records from the top of the file
    while (std::getline(file, line) && result.counter < max_chunk_size)
    {
        std::stringstream ss(line);
        std::string value;
        TelemetryRecord record;
        
        int metric = 0;
        while (std::getline (ss, value, ','))
        {
            switch (metric)
            {
                case 0:
                    record.id = stoi(value);
                    break;

                case 1:
                    record.timestamp = stoll(value);
                    break;

                case 2:
                    record.rpm = stod(value);
                    break;

                case 3:
                    record.speed = stod(value);
                    break;

                case 4:
                    record.coolantTemp = stod(value);
                    break;

                default:
                    break;
            }
            metric++;
        }
        result.data.push_back(record);
        result.counter++;
    }
    file.close();

    return result;
}

/**
 * @brief Deletes successfully transmitted records from the beginning of telemetry.csv.
 */
void remove_sent_data(int counter)
{
    std::ifstream file("telemetry.csv");

    std::string data_to_rewrite;
    std::string line;

    // Skip the first 'counter' lines that were successfully published
    for (int i = 0; i < counter && std::getline(file, line); i++)
    {
    }

    // Accumulate the remaining unsent lines
    while (std::getline(file, line))
    {
        data_to_rewrite += line + "\n";
    }

    file.close();

    // Overwrite the file with only the remaining un-transmitted data
    std::ofstream file_out("telemetry.csv", std::ios::trunc);
    file_out << data_to_rewrite;
    file_out.close();
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