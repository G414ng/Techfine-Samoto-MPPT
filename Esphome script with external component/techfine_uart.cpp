#include "techfine_uart.h"
#include "esphome/core/log.h"
#include <vector>

namespace techfine_uart {

static const char *TAG = "techfine_uart";

TechfineUART::TechfineUART(esphome::uart::UARTComponent *parent)
    : PollingComponent(3000), UARTDevice(parent) {}

void TechfineUART::set_sensors(
    esphome::sensor::Sensor *pv,
    esphome::sensor::Sensor *batt,
    esphome::sensor::Sensor *cur,
    esphome::sensor::Sensor *mcu,
    esphome::sensor::Sensor *hs,
    esphome::sensor::Sensor *day,
    esphome::sensor::Sensor *tot,
    esphome::sensor::Sensor *pwr
) {
  pv_voltage_ = pv;
  batt_voltage_ = batt;
  charge_current_ = cur;
  mcu_temp_ = mcu;
  heatsink_temp_ = hs;
  daily_kwh_ = day;
  total_kwh_ = tot;
  pv_power_ = pwr;
}

void TechfineUART::setup() {
  ESP_LOGI(TAG, "Techfine UART initialized");
}

uint8_t TechfineUART::get_b(std::vector<uint8_t> &buf, int header, int pos) {
  int idx = header + (pos - 1);
  if (idx < 0 || idx >= (int)buf.size()) return 0;
  return buf[idx];
}

void TechfineUART::update() {
  const uint8_t cmd[] = {0x43,0x4D,0x48,0x42,0x06,0x4D,0x50,0x50,0x54,0x48,0x42};
  this->write_array(cmd, sizeof(cmd));
  this->flush();

  esphome::delay(150);

  std::vector<uint8_t> buf;
  uint32_t start = esphome::millis();

  while (this->available() && buf.size() < 128) {
    int v = this->read();
    if (v >= 0) buf.push_back((uint8_t)v);
    if (esphome::millis() - start > 500) break;
  }

  if (buf.empty()) {
    ESP_LOGW(TAG, "No UART data received");
    return;
  }

  int header = -1;
  for (int i = 0; i + 4 < (int)buf.size(); i++) {
    if (buf[i] == 0x4D && buf[i+1] == 0x56 && buf[i+2] == 0x4D &&
        buf[i+3] == 0x50 && buf[i+4] == 0x39) {
      header = i;
      break;
    }
  }

  if (header < 0) {
    ESP_LOGW(TAG, "Header not found, setting values to zero");
    if (pv_voltage_) pv_voltage_->publish_state(0);
    if (batt_voltage_) batt_voltage_->publish_state(0);
    if (charge_current_) charge_current_->publish_state(0);
    if (mcu_temp_) mcu_temp_->publish_state(0);
    if (heatsink_temp_) heatsink_temp_->publish_state(0);
    if (daily_kwh_) daily_kwh_->publish_state(0);
    if (total_kwh_) total_kwh_->publish_state(0);
    if (pv_power_) pv_power_->publish_state(0);
    return;
  }

  // Decode fields (same as before)
  uint16_t pv_voltage = get_b(buf, header, 16);

  uint16_t batt_hi = get_b(buf, header, 21);
  uint16_t batt_lo = get_b(buf, header, 20);
  float batt_voltage = ((batt_hi << 8) | batt_lo) / 10.0f;

  uint16_t charge_current = get_b(buf, header, 24);
  uint16_t mcu_temp = get_b(buf, header, 25);

  uint16_t hs_hi = get_b(buf, header, 27);
  uint16_t hs_lo = get_b(buf, header, 26);
  uint16_t heatsink_temp = (hs_hi << 8) | hs_lo;

  uint16_t day_int = (get_b(buf, header, 29) << 8) | get_b(buf, header, 28);
  uint16_t day_frac = (get_b(buf, header, 31) << 8) | get_b(buf, header, 30);
  float daily_kwh = day_int + (day_frac / 1000.0f);

  uint16_t total_int = (get_b(buf, header, 35) << 8) | get_b(buf, header, 34);
  uint16_t total_frac = get_b(buf, header, 33);
  float total_kwh = total_int + (total_frac / 10.0f);

  float pv_power = batt_voltage * charge_current;

  if (pv_voltage_) pv_voltage_->publish_state(pv_voltage);
  if (batt_voltage_) batt_voltage_->publish_state(batt_voltage);
  if (charge_current_) charge_current_->publish_state(charge_current);
  if (mcu_temp_) mcu_temp_->publish_state(mcu_temp);
  if (heatsink_temp_) heatsink_temp_->publish_state(heatsink_temp);
  if (daily_kwh_) daily_kwh_->publish_state(daily_kwh);
  if (total_kwh_) total_kwh_->publish_state(total_kwh);
  if (pv_power_) pv_power_->publish_state(pv_power);
}

}  // namespace techfine_uart
