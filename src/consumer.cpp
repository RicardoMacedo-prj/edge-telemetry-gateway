#include "consumer.hpp"
#include <iostream>
#include "decoder.hpp"
#include "rules.hpp"
#include "mqtt_client.hpp"
#include "storage.hpp"

void CanProcessorLoop(mosquitto* mosq, bool& is_connected, ThreadSafeQueue& queue, 
                    GatewayConfig config, std::atomic<bool>& running)
{
    // Buffer to hold incoming 8-byte CAN frame
    CanFrame canFrameToProcess;

    // Load highest record ID once at startup to keep sequential auto-incrementing in memory
    long long max_id = get_max_id();
    if (max_id == -1)
    {
        std::cout << "[Decoder] No records found in telemetry.csv" << std::endl;
        max_id = 0;
    }


    while(running)
    {
        // Get a CanFrame to process
        canFrameToProcess = queue.pop(running);

        // Decode incoming CAN frame into timestamp, metric value, and PID type
        auto [timestamp, data, PID] = decode_can(canFrameToProcess);

        // Ignore unrecognized or unsupported sensor PIDs
        if (PID == PID_standart::unknown)
        {
            continue;
        }

        // Evaluate edge rules and publish alert if safety threshold was violated
        auto [is_alert, alert_type, threshold] = check_alert(PID, data);
        if (is_alert)
        {
            send_alert_telemetry(mosq, data, alert_type, threshold, timestamp);
        }

        bool save_ok = false;

        // Route decoded value into local CSV storage based on PID type
        if (canFrameToProcess.data[1] == static_cast<uint8_t>(PID_standart::rpm))
        {   
            save_ok = save_telemetry_csv(
                max_id += 1,
                timestamp,
                data,
                0.0,
                0.0,
                config.MAX_STORAGE_RECORDS);
        }
        else if (canFrameToProcess.data[1] == static_cast<uint8_t>(PID_standart::speed))
        {
            save_ok = save_telemetry_csv(
                max_id += 1,
                timestamp,
                0.0,
                data,
                0.0,
                config.MAX_STORAGE_RECORDS);
        }
        else if (canFrameToProcess.data[1] == static_cast<uint8_t>(PID_standart::coolantTemp))
        {
            save_ok = save_telemetry_csv(
                max_id += 1,
                timestamp,
                0.0,
                0.0,
                data,
                config.MAX_STORAGE_RECORDS);
        }

        // Store-and-Forward: flush buffered records to broker only if successfully buffered and connected
        if (!save_ok) {
            std::cerr << "[Storage] An error occurred while saving telemetry data." << std::endl;
        } else if (is_connected) {
            send_telemetry(mosq, config.MAX_CHUNK_SIZE);
        }
    }
}