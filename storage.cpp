#include <iostream>
#include <fstream>
#include <string>

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

int main()
{
    // Example telemetry data
    int id = 1;
    long long timestamp = 1622547800;
    double rpm = 2500.0;
    double speed = 85.0;
    double coolantTemp = 88.0;

    // Test saving telemetry record
    if (save_telemetry_csv(id, timestamp, rpm, speed, coolantTemp))
    {
        std::cout << "Telemetry data saved successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Failed to save telemetry data." << std::endl;
    }

    read_telemetry_csv();  // Read and display stored telemetry
    clear_telemetry_csv(); // Clean buffer after verification

    return 0;
}