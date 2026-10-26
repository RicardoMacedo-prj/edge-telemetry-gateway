#include "rules.hpp"

/**
 * @brief Checks if a sensor metric exceeds safe operating limits.
 */
std::tuple<bool, std::string, double> check_alert(PID_standart pid, double data)
{
    bool is_alert = false;
    std::string alert_type; 
    double threshold = 0.0;

    // Warn if engine RPM exceeds 4000
    if (pid == PID_standart::rpm)
    {
        if (data > 4000.0)
        {
            is_alert = true;
            alert_type = "HIGH_RPM";
            threshold = 4000.0;
        }
    }
    // Warn if vehicle speed exceeds 120 km/h
    else if (pid == PID_standart::speed)
    {
        if (data > 120.0)
        {
            is_alert = true;
            alert_type = "OVERSPEED";
            threshold = 120.0;
        }
    }
    // Warn if coolant temperature exceeds 100 C
    else if (pid == PID_standart::coolantTemp)
    {
        if (data > 100.0)
        {
            is_alert = true;
            alert_type = "ENGINE_OVERHEAT";
            threshold = 100.0;
        }
    }

    return {is_alert, alert_type, threshold};
}

