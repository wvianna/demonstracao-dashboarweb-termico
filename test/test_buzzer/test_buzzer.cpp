// Testes HOST do agendador do buzzer (lib/thermocore).
// Nivel: HOST. Roda com: pio test -e native
#include <unity.h>
#include "buzzer.h"

using namespace thermocore;

void setUp() {}
void tearDown() {}

void test_temp_alarm_cycle_150_2000() {
    Buzzer b;
    b.setMode(BuzzerMode::ALARM_TEMP);
    TEST_ASSERT_TRUE(b.output()); // inicia ON
    b.tick(1000);                 // ancora fase em 1000
    TEST_ASSERT_TRUE(b.output()); // dentro dos 150ms
    b.tick(1000 + Buzzer::kOnTempMs);
    TEST_ASSERT_FALSE(b.output()); // entrou em OFF
    b.tick(1000 + Buzzer::kOnTempMs + Buzzer::kOffTempMs);
    TEST_ASSERT_TRUE(b.output()); // voltou a ON (ciclo fechado)
}

void test_fail_sensor_cycle_300_5000() {
    Buzzer b;
    b.setMode(BuzzerMode::FAIL_SENSOR);
    b.tick(5000);
    TEST_ASSERT_TRUE(b.output());
    b.tick(5000 + Buzzer::kOnFailMs);
    TEST_ASSERT_FALSE(b.output());
    b.tick(5000 + Buzzer::kOnFailMs + Buzzer::kOffFailMs);
    TEST_ASSERT_TRUE(b.output());
}

void test_none_off() {
    Buzzer b;
    b.setMode(BuzzerMode::NONE);
    b.tick(0);
    TEST_ASSERT_FALSE(b.output());
}

void test_mode_change_resets_phase() {
    Buzzer b;
    b.setMode(BuzzerMode::ALARM_TEMP);
    b.tick(1000);
    TEST_ASSERT_TRUE(b.output());
    b.setMode(BuzzerMode::FAIL_SENSOR); // reinicia ciclo em ON
    b.tick(2000);
    TEST_ASSERT_TRUE(b.output());
    b.tick(2000 + Buzzer::kOnFailMs);
    TEST_ASSERT_FALSE(b.output());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_temp_alarm_cycle_150_2000);
    RUN_TEST(test_fail_sensor_cycle_300_5000);
    RUN_TEST(test_none_off);
    RUN_TEST(test_mode_change_resets_phase);
    return UNITY_END();
}
