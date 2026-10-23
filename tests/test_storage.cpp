#include <cassert>
#include <string>
#include "storage.hpp"


void test_fifo_eviction()
{
    clear_telemetry_csv();

    // Max capacity of 3 records
    int max_storage = 3;
    save_telemetry_csv(1, 1000, 1000.0, 10.0, 70.0, max_storage);
    save_telemetry_csv(2, 2000, 2000.0, 20.0, 75.0, max_storage);
    save_telemetry_csv(3, 3000, 3000.0, 30.0, 80.0, max_storage);
    assert(count_csv_records() == 3);

    // Saving 4th record must evict ID 1
    save_telemetry_csv(4, 4000, 4000.0, 40.0, 85.0, max_storage);
    assert(count_csv_records() == 3);

    // Verify oldest ID 1 was evicted and remaining are 2, 3, 4
    auto result = get_data_to_publish(10);
    assert(result.counter == 3);
    assert(result.data[0].id == 2);
    assert(result.data[1].id == 3);
    assert(result.data[2].id == 4);

    clear_telemetry_csv();
}

void test_chunk_publish_and_remove()
{
    clear_telemetry_csv();

    for (int i = 1; i <= 5; ++i)
    {
        save_telemetry_csv(i, i * 1000, 2000.0, 50.0, 80.0, 10);
    }
    assert(count_csv_records() == 5);
    assert(get_max_id() == 5);

    // Retrieve chunk of 3 records
    auto result = get_data_to_publish(3);
    assert(result.counter == 3);

    // Remove the 3 sent records
    remove_sent_data(result.counter);
    assert(count_csv_records() == 2);

    // Remaining records must be 4 and 5
    auto remaining_result = get_data_to_publish(10);
    assert(remaining_result.counter == 2);
    assert(remaining_result.data[0].id = 4);
    assert(remaining_result.data[1].id = 5);

    clear_telemetry_csv();
}

int main()
{
    test_fifo_eviction();
    test_chunk_publish_and_remove();

    return 0;
}
