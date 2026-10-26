#pragma once

#include <tuple>
#include <string>
#include "vehicle.hpp"

/**
 * @brief Decodes an 8-byte OBD-II CAN frame into a timestamp and sensor metric.
 * 
 * Inspects byte 1 to determine the PID type (RPM, Speed, or Coolant Temperature)
 * and reconstructs the physical engineering value according to the SAE J1979 standard.
 * 
 * @param canFrame The 8-byte CAN message to decode.
 * @return A tuple containing:
 *         - first: Timestamp in milliseconds since Unix epoch.
 *         - second: Decoded numerical metric value.
 *         - third: Decoded sensor PID type.
 */
std::tuple<long long, double, PID_standart> decode_can(CanFrame canFrame);
