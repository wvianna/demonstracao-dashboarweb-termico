// Testes HOST da FSM de seguranca/controle (lib/thermocore).
// Nivel: HOST. Roda com: pio test -e native
#include <unity.h>
#include "thermal_fsm.h"

using namespace thermocore;

void setUp() {}
void tearDown() {}

static ThermalFsm fsm;
static void reset() { fsm = ThermalFsm(); fsm.begin(); }

void test_boot_sensor_found() {
    reset();
    TEST_ASSERT_EQUAL_INT((int)ThermalState::SCAN_ONEWIRE, (int)fsm.output().state);
    fsm.onSensorFound();
    TEST_ASSERT_EQUAL_INT((int)ThermalState::MONITORING, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_TRUE(fsm.output().sensorPresent);
}

void test_boot_sensor_missing_blocks_load() {
    reset();
    fsm.onSensorMissing();
    TEST_ASSERT_EQUAL_INT((int)ThermalState::SAFE_STOP, (int)fsm.output().state);
    // CA-SNS-001: sem sensor, ON e rejeitado
    TEST_ASSERT_FALSE(fsm.requestOn());
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_FALSE(fsm.output().loadRequested);
}

void test_on_off_normal() {
    reset();
    fsm.onSensorFound();
    fsm.onSample(true, 25.0f);
    TEST_ASSERT_TRUE(fsm.requestOn());
    TEST_ASSERT_EQUAL_INT(ThermalFsm::kPwmMax, fsm.output().pwm);
    TEST_ASSERT_EQUAL_INT((int)ThermalState::HEATER_ON, (int)fsm.output().state);
    fsm.requestOff();
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_EQUAL_INT((int)ThermalState::MONITORING, (int)fsm.output().state);
}

void test_high_temp_alarm_and_recovery() {
    reset();
    fsm.onSensorFound();
    fsm.onSample(true, 25.0f);
    TEST_ASSERT_TRUE(fsm.requestOn());
    TEST_ASSERT_EQUAL_INT(ThermalFsm::kPwmMax, fsm.output().pwm);

    fsm.onSample(true, 81.0f); // >= 80 C -> desarme imediato (FR-SAF-001)
    TEST_ASSERT_EQUAL_INT((int)ThermalState::ALARM_TEMP, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_EQUAL_INT((int)BuzzerMode::ALARM_TEMP, (int)fsm.output().buzzer);
    TEST_ASSERT_EQUAL_INT((int)AlarmKind::TEMP_HIGH, (int)fsm.output().alarm);
    // CA-SAF-002: ON rejeitado durante risco
    TEST_ASSERT_FALSE(fsm.requestOn());
    // CA-SAF-003/004: risco cessa; buzzer para; carga permanece OFF
    fsm.onSample(true, 60.0f);
    TEST_ASSERT_EQUAL_INT((int)ThermalState::MONITORING, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT((int)BuzzerMode::NONE, (int)fsm.output().buzzer);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
}

void test_sensor_fail_and_recovery() {
    reset();
    fsm.onSensorFound();
    fsm.onSample(true, 25.0f);
    TEST_ASSERT_TRUE(fsm.requestOn());
    fsm.onSample(false, 0.0f); // falha de comunicacao (FR-SAF-002)
    TEST_ASSERT_EQUAL_INT((int)ThermalState::FAIL_SENSOR, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_EQUAL_INT((int)BuzzerMode::FAIL_SENSOR, (int)fsm.output().buzzer);
    TEST_ASSERT_EQUAL_INT((int)AlarmKind::SENSOR_FAIL, (int)fsm.output().alarm);
    // CA-SNS-004: leitura valida retoma monitoramento; carga OFF
    fsm.onSample(true, 30.0f);
    TEST_ASSERT_EQUAL_INT((int)ThermalState::MONITORING, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT((int)BuzzerMode::NONE, (int)fsm.output().buzzer);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
}

void test_interlock_on_at_exact_threshold() {
    reset();
    fsm.onSensorFound();
    fsm.onSample(true, 79.9f);
    TEST_ASSERT_TRUE(fsm.requestOn()); // 79.9 < 80 -> ok
    fsm.onSample(true, 80.0f);         // exatamente 80 -> desarme
    TEST_ASSERT_EQUAL_INT((int)ThermalState::ALARM_TEMP, (int)fsm.output().state);
    TEST_ASSERT_EQUAL_INT(0, fsm.output().pwm);
    TEST_ASSERT_FALSE(fsm.requestOn());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_boot_sensor_found);
    RUN_TEST(test_boot_sensor_missing_blocks_load);
    RUN_TEST(test_on_off_normal);
    RUN_TEST(test_high_temp_alarm_and_recovery);
    RUN_TEST(test_sensor_fail_and_recovery);
    RUN_TEST(test_interlock_on_at_exact_threshold);
    return UNITY_END();
}
