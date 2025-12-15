#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace techfine_uart {

class TechfineUART : public esphome::PollingComponent, public esphome::uart::UARTDevice {
 public:
  explicit TechfineUART(esphome::uart::UARTComponent *parent);

  void setup() override;
  void update() override;

  void set_sensors(
      esphome::sensor::Sensor *pv,
      esphome::sensor::Sensor *batt,
      esphome::sensor::Sensor *cur,
      esphome::sensor::Sensor *mcu,
      esphome::sensor::Sensor *hs,
      esphome::sensor::Sensor *day,
      esphome::sensor::Sensor *tot,
      esphome::sensor::Sensor *pwr
  );

 protected:
  uint8_t get_b(std::vector<uint8_t> &buf, int header, int pos);

  esphome::sensor::Sensor *pv_voltage_{nullptr};
  esphome::sensor::Sensor *batt_voltage_{nullptr};
  esphome::sensor::Sensor *charge_current_{nullptr};
  esphome::sensor::Sensor *mcu_temp_{nullptr};
  esphome::sensor::Sensor *heatsink_temp_{nullptr};
  esphome::sensor::Sensor *daily_kwh_{nullptr};
  esphome::sensor::Sensor *total_kwh_{nullptr};
  esphome::sensor::Sensor *pv_power_{nullptr};
};

}  // namespace techfine_uart
