// All GPIO/PMIC/sleep details stay here; gameplay uses the portable PowerPolicy.
#include <Arduino.h>
#include <M5Unified.h>
#include "Power.h"
#ifndef NOURS_DESKTOP
#include <esp_sleep.h>
#include <driver/gpio.h>
#endif

void Power::begin() {
#ifndef NOURS_DESKTOP
    // Keep boost alive during light sleep so RAM and the Study session survive.
    if (M5.Power.getType() == m5::Power_Class::pmic_t::pmic_ip5306)
        M5.Power.Ip5306.setPowerBoostKeepOn(true);
    M5.Display.setBrightness(160);
#endif
    polledAt_ = millis() - 1000;
    poll(millis());
}

// FIRE lacks a VBUS voltmeter. Charge/full flags conservatively indicate external
// power; an I2C failure is UNKNOWN, never proof that the device is on battery.
void Power::poll(uint32_t now) {
    if (now - polledAt_ < 1000) return;
    polledAt_ = now;
#ifdef NOURS_DESKTOP
    source_ = PowerSource::External;
#else
    uint8_t charging = 0, full = 0;
    if (M5.Power.getType() == m5::Power_Class::pmic_t::pmic_ip5306 &&
        M5.In_I2C.readRegister(0x75, 0x70, &charging, 1, 100000) &&
        M5.In_I2C.readRegister(0x75, 0x71, &full, 1, 100000)) {
        source_ = ((charging | full) & 0x08) ? PowerSource::External : PowerSource::Battery;
    } else {
        source_ = PowerSource::Unknown;
    }
#endif
}

const char* Power::sourceLabel() const {
    return source_ == PowerSource::External ? "USB/Full" :
           source_ == PowerSource::Battery ? "Battery" : "Unknown";
}

void Power::brightness(IdleState state) {
    if (brightness_ == state) return;
    brightness_ = state;
#ifndef NOURS_DESKTOP
    M5.Display.setBrightness(state == IdleState::Awake ? 160 : state == IdleState::Dim ? 24 : 0);
#endif
}

// One bounded low-power slice; timer wake services Study and supply changes.
// A/B/C are active-low FIRE pins. No deep sleep means no reboot on normal wake.
void Power::sleepSlice() {
#ifndef NOURS_DESKTOP
    const gpio_num_t pins[] = {GPIO_NUM_39, GPIO_NUM_38, GPIO_NUM_37};
    for (auto pin : pins)
        if (gpio_get_level(pin) == 0) return; // Let Input debounce the wake press.
    for (auto pin : pins) {
        gpio_wakeup_enable(pin, GPIO_INTR_LOW_LEVEL);
    }
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(250000);
    esp_light_sleep_start();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    for (auto pin : pins) gpio_wakeup_disable(pin);
#endif
}

// Clear every timer/GPIO wake source before shutdown, eliminating instant wake.
// USB cannot physically be cut by FIRE: deep sleep remains until power/reset.
void Power::shutdown() {
#ifndef NOURS_DESKTOP
    M5.Speaker.end();
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    if (M5.Power.getType() == m5::Power_Class::pmic_t::pmic_ip5306)
        M5.Power.Ip5306.setPowerBoostKeepOn(false);
    esp_deep_sleep_start();
#endif
}
