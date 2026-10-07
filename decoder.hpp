#pragma once

#include <utility>
#include <string>
#include "car.hpp"

/**
 * @brief Decodes an 8-byte OBD-II CAN frame into a timestamp and sensor metric.
 * 
 * Inspects byte 1 to determine the PID type (RPM, Speed, or Coolant Temperature)
 * and reconstructs the physical engineering value according to the SAE J1979 standard.
 * 
 * @param canFrame The 8-byte CAN message to decode.
 * @return A pair containing:
 *         - first: Timestamp in milliseconds since Unix epoch.
 *         - second: Decoded numerical metric value.
 */
std::pair<long long, double> decode_can(CanFrame canFrame);
