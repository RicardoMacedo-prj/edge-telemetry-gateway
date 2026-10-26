#include <cassert>
#include <tuple>
#include "rules.hpp"
#include "car.hpp"

void test_rpm_rules()
{
    // Normal RPM should not trigger an alert
    auto [alert_normal, type_normal, th_normal] = check_alert(PID_standart::rpm, 2500.0);
    assert(alert_normal == false);

    // RPM exceeding 4000 should trigger a HIGH_RPM alert
    auto [alert_high, type_high, th_high] = check_alert(PID_standart::rpm, 4200.0);
    assert(alert_high == true);
    assert(type_high == "HIGH_RPM");
    assert(th_high == 4000.0);
}

void test_speed_rules()
{
    // Normal speed should not trigger an alert
    auto [alert_normal, type_normal, th_normal] = check_alert(PID_standart::speed, 90.0);
    assert(alert_normal == false);

    // Speed exceeding 120 km/h should trigger an OVERSPEED alert
    auto [alert_high, type_high, th_high] = check_alert(PID_standart::speed, 130.0);
    assert(alert_high == true);
    assert(type_high == "OVERSPEED");
    assert(th_high == 120.0);
}

void test_coolant_rules()
{
    // Normal coolant temperature should not trigger an alert
    auto [alert_normal, type_normal, th_normal] = check_alert(PID_standart::coolantTemp, 85.0);
    assert(alert_normal == false);

    // Coolant temperature exceeding 100 C should trigger an ENGINE_OVERHEAT alert
    auto [alert_high, type_high, th_high] = check_alert(PID_standart::coolantTemp, 105.0);
    assert(alert_high == true);
    assert(type_high == "ENGINE_OVERHEAT");
    assert(th_high == 100.0);
}

void test_unknown_pid()
{
    // Unrecognized or unsupported PID should never trigger an alert
    auto [alert, type, th] = check_alert(PID_standart::unknown, 5000.0);
    assert(alert == false);
}

int main()
{
    test_rpm_rules();
    test_speed_rules();
    test_coolant_rules();
    test_unknown_pid();

    return 0;
}
