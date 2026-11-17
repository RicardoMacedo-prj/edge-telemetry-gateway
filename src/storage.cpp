#include "storage.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sqlite3.h>

static sqlite3* db = nullptr;

void init_storage(const std::string& file)
{
    if (db != nullptr)
    {
        return;
    }

    int rc = sqlite3_open(file.c_str(), &db);

    if (rc != SQLITE_OK)
    {
        std::cerr << "[Storage] Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS telemetry ("
        "    id INTEGER PRIMARY KEY,"
        "    timestamp INTEGER NOT NULL,"
        "    rpm REAL NOT NULL,"
        "    speed REAL NOT NULL,"
        "    coolant_temp REAL NOT NULL"
        ");"
        "PRAGMA journal_mode=WAL;";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "[Storage] SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void close_storage()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

/**
 * @brief Counts the total number of telemetry records stored in the database.
 * @return Number of records stored in the database, or -1 on error.
 */
int count_csv_records()
{
    init_storage();

    const char* sql = "SELECT COUNT(*) FROM telemetry;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

/**
 * @brief Inserts a telemetry record into the database table, enforcing a FIFO limit.
 */
bool save_telemetry_csv(int id, long long timestamp, double rpm, double speed, double coolantTemp, int max_storage_records)
{
    init_storage();

    // FIFO eviction: if buffer is at capacity, discard the oldest record
    if (count_csv_records() >= max_storage_records)
    {
        std::cerr << "Maximum record limit reached. Evicting oldest record." << std::endl;

        const char* sqlDel =
        "DELETE FROM telemetry "
        "WHERE id IN ("
        "    SELECT id FROM telemetry "
        "    ORDER BY id ASC "
        "    LIMIT 1"
        ");";

        char* errMsgDel = nullptr;
        int rc = sqlite3_exec(db, sqlDel, nullptr, nullptr, &errMsgDel);
        if (rc != SQLITE_OK)
        {
            std::cerr << "[Storage] SQL error: " << errMsgDel << std::endl;
            sqlite3_free(errMsgDel);
            return false;
        }
    }

    const char* sqlAdd =
    "INSERT INTO telemetry (id, timestamp, rpm, speed, coolant_temp) " 
    "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlAdd, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int64(stmt, 2, timestamp);
    sqlite3_bind_double(stmt, 3, rpm);
    sqlite3_bind_double(stmt, 4, speed);
    sqlite3_bind_double(stmt, 5, coolantTemp);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        std::cerr << "[Storage] Error on database insertion: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Reads and displays all stored records from the database table.
 */
void read_telemetry_csv()
{
    init_storage();

    const char* sql = "SELECT * FROM telemetry;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << 
            sqlite3_column_int(stmt, 0) << ", " <<
            sqlite3_column_int64(stmt, 1) << ", " <<
            sqlite3_column_double(stmt, 2) << ", " <<
            sqlite3_column_double(stmt, 3) << ", " <<
            sqlite3_column_double(stmt, 4) << std::endl;
    }

    sqlite3_finalize(stmt);
}

/**
 * @brief Retrieves a batch of up to max_chunk_size records from the database to prepare an MQTT batch.
 */
DataToPublish get_data_to_publish(int max_chunk_size)
{
    init_storage();

    DataToPublish result;
    int counter = 0;

    const char* sql =
        "SELECT id, timestamp, rpm, speed, coolant_temp "
        "FROM telemetry "
        "ORDER BY id ASC "
        "LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    sqlite3_bind_int(stmt, 1, max_chunk_size);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        TelemetryRecord record;

        record.id = sqlite3_column_int(stmt, 0);
        record.timestamp = sqlite3_column_int64(stmt, 1);
        record.rpm = sqlite3_column_double(stmt, 2);
        record.speed = sqlite3_column_double(stmt, 3);
        record.coolantTemp = sqlite3_column_double(stmt, 4);
        counter++;

        result.data.push_back(record);
    }

    sqlite3_finalize(stmt);
    result.counter = counter;
    return result;
}

/**
 * @brief Deletes successfully transmitted records from the database table.
 */
void remove_sent_data(int counter)
{
    init_storage();

    const char* sql =
    "DELETE FROM telemetry "
    "WHERE id IN ("
    "    SELECT id FROM telemetry "
    "    ORDER BY id ASC "
    "    LIMIT ?"
    ");";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, counter);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

/**
 * @brief Queries the database to determine the highest existing record ID.
 * @return Highest record ID found, or -1 if the table is empty or on error.
 */
long long get_max_id()
{
    init_storage();

    long long max_id = -1;

    const char* sql = "SELECT COALESCE(MAX(id), -1) FROM telemetry;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "[Storage] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        max_id = sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return max_id;
}

/**
 * @brief Clears all records from the telemetry database table.
 */
void clear_telemetry_csv()
{
    init_storage();
    
    const char* sql =
    "DELETE FROM telemetry;";

    char* errMsg = nullptr;
    
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK)
    {
        std::cerr << "[Storage] SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}