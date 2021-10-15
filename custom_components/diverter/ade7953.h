#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace ade7953 {

static const int32_t COUNT_10J = 128; //=10*207024/4862401*power_factor

class ADE7953 : public i2c::I2CDevice, public PollingComponent {
 public:
  void set_irq_pin(GPIOPin *irq_pin) { this->irq_pin_ = irq_pin; }
  void set_load_pin(GPIOPin *load_pin) { this->load_pin_ = load_pin; }

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_a_sensor(sensor::Sensor *current_a_sensor) { current_a_sensor_ = current_a_sensor; }
  void set_current_b_sensor(sensor::Sensor *current_b_sensor) { current_b_sensor_ = current_b_sensor; }
  void set_active_power_a_sensor(sensor::Sensor *active_power_a_sensor) {
    active_power_a_sensor_ = active_power_a_sensor;
  }
  void set_active_power_b_sensor(sensor::Sensor *active_power_b_sensor) {
    active_power_b_sensor_ = active_power_b_sensor;
  }
  void set_energy_buffer_sensor(sensor::Sensor *energy_buffer_sensor) {
    energy_buffer_sensor_ = energy_buffer_sensor;
  }
  void set_energy_diverted_sensor(sensor::Sensor *energy_diverted_sensor) {
    energy_diverted_sensor_ = energy_diverted_sensor;
  }
  
  void set_diverter_parameters(uint32_t energy_buffer_size, float energy_threshold_high_percent, float energy_threshold_low_percent) {
    energy_buffer_size_=energy_buffer_size*COUNT_10J/10;
    energy_threshold_high_=energy_buffer_size_*energy_threshold_high_percent;
    energy_threshold_low_=energy_buffer_size_*energy_threshold_low_percent;
    if (energy_threshold_low_>energy_threshold_high_) energy_threshold_low_=energy_threshold_high_;
  }


  void setup() override {
    this->irq_pin_->setup();
    this->load_pin_->setup();
    this->load_pin_->digital_write(false);

    this->set_timeout(100, [this]() {
      this->ade_write_<uint8_t>(0x0102, 0x04); //lock interface, keep HPFEN 
      this->ade_write_<uint8_t>(0x00FE, 0xAD); //unlock next command
      this->ade_write_<uint16_t>(0x0120, 0x0030);  //required register setting
      this->ade_write_<uint8_t>(0x0001, 0x01); //disable the active power no-load feature
      this->ade_write_<uint8_t>(0x0004, 0x01); //Enable ALWATT - active energy line cycle accumulation
      this->ade_write_<uint16_t>(0x0101, 0x001); //Accumulate 1 cycle 
      this->ade_write_<uint32_t>(0x032C, 0x00140000); //Enable CYCEND Interrupt
      this->ade_read_<uint32_t>(0x032E); //reset interrupts      
      this->is_setup_ = true;
    });
  }

  void dump_config() override;

  void update() override;

  void loop() override;

 protected:
  template<typename T> bool ade_write_(uint16_t reg, T value) {
    std::vector<uint8_t> data;
    data.push_back(reg >> 8);
    data.push_back(reg >> 0);
    for (int i = sizeof(T) - 1; i >= 0; i--)
      data.push_back(value >> (i * 8));
    return this->write_bytes_raw(data);
  }
  template<typename T> optional<T> ade_read_(uint16_t reg) {
    uint8_t hi = reg >> 8;
    uint8_t lo = reg >> 0;
    if (!this->write_bytes_raw({hi, lo}))
      return {};
    auto ret = this->read_bytes_raw<sizeof(T)>();
    if (!ret.has_value())
      return {};
    T result = 0;
    for (int i = 0, j = sizeof(T) - 1; i < sizeof(T); i++, j--)
      result |= T((*ret)[i]) << (j * 8);
    return result;
  }

  GPIOPin *irq_pin_;
  GPIOPin *load_pin_;
  
  bool  load_status_ = false;

  int32_t energy_buffer_ = 0;
  uint32_t energy_diverted_ = 0;
  uint32_t energy_buffer_size_;
  uint32_t energy_threshold_high_;
  uint32_t energy_threshold_low_;

  bool is_setup_{false};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_a_sensor_{nullptr};
  sensor::Sensor *current_b_sensor_{nullptr};
  sensor::Sensor *active_power_a_sensor_{nullptr};
  sensor::Sensor *active_power_b_sensor_{nullptr};
  sensor::Sensor *energy_buffer_sensor_{nullptr};
  sensor::Sensor *energy_diverted_sensor_{nullptr};
};

}  // namespace ade7953
}  // namespace esphome
