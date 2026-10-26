#pragma once

#include <iostream>
#include <string>
#include "vehicle.hpp"

/**
 * @brief Evaluates whether a telemetry metric exceeds critical safety thresholds.
 *
 * Checks physical values (RPM, Speed, Coolant Temperature) against edge limits.
 *
 * @param pid OBD-II sensor identifier.
 * @param data Decoded physical metric value.
 * @return A tuple containing:
 *         - bool: true if a safety threshold was violated, false otherwise.
 *         - std::string: Alert type descriptor (e.g., "HIGH_RPM", "OVERSPEED").
 *         - double: The violated threshold limit value.
 */
std::tuple<bool, std::string, double> check_alert(PID_standart pid, double data);