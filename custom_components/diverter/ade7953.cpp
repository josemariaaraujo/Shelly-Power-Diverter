#include "ade7953.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ade7953 {

static const char *TAG = "ade7953";




void ADE7953::dump_config() {
  ESP_LOGCONFIG(TAG, "ADE7953:");
  ESP_LOGCONFIG(TAG, "  IRQ Pin: GPIO%u", this->irq_pin_->get_pin());
  ESP_LOGCONFIG(TAG, "  SWITCH Pin: GPIO%u", this->load_pin_->get_pin());
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Voltage Sensor", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current A Sensor", this->current_a_sensor_);
  LOG_SENSOR("  ", "Current B Sensor", this->current_b_sensor_);
  LOG_SENSOR("  ", "Active Power A Sensor", this->active_power_a_sensor_);
  LOG_SENSOR("  ", "Active Power B Sensor", this->active_power_b_sensor_);
  LOG_SENSOR("  ", "Energy Buffer Sensor", this->energy_buffer_sensor_);
}

#define ADE_PUBLISH_(name, factor) \
  if (name && this->name##_sensor_) { \
    float value = *name / factor; \
    this->name##_sensor_->publish_state(value); \
  }
#define ADE_PUBLISH(name, factor) ADE_PUBLISH_(name, factor)

void ADE7953::update() {
  if (!this->is_setup_)
    return;

  auto energy_buffer = &this->energy_buffer_ ;
  ADE_PUBLISH(energy_buffer, 1.0f);
  auto energy_diverted = &this->energy_diverted_ ;
  ADE_PUBLISH(energy_diverted, 1.0f);  
  auto active_power_a = this->ade_read_<int32_t>(0x0312);
  ADE_PUBLISH(active_power_a, 300.0f);    //f=4862401*current_factor*voltage_factor/(9032007*9032007)
  auto active_power_b = this->ade_read_<int32_t>(0x0313);
  ADE_PUBLISH(active_power_b, 300.0f);
  auto current_a = this->ade_read_<uint32_t>(0x031A);
  ADE_PUBLISH(current_a, 193700.0f);  // adjusted, f=9032007/(0,5*ratio_CT/(R*SQRT(2))
  auto current_b = this->ade_read_<uint32_t>(0x031B);
  ADE_PUBLISH(current_b, 193700.0f);
  auto voltage = this->ade_read_<uint32_t>(0x031C);
  ADE_PUBLISH(voltage, 26000.0f);
}

void ADE7953::loop() {
  if (!irq_pin_->digital_read()) {
    this->ade_read_<uint32_t>(0x032E); //reset interrupts
    energy_buffer_+=*this->ade_read_<int32_t>(0x031E);
    if (energy_buffer_<0) energy_buffer_=0;
    if (energy_buffer_>energy_buffer_size_) energy_buffer_=energy_buffer_size_;
    if (load_status_==false && energy_buffer_>energy_threshold_high_){
        load_status_=true;
        load_pin_->digital_write(true);
        energy_diverted_=1; //++;
    } else if (load_status_==true && energy_buffer_<energy_threshold_low_){
        load_status_=false;
        load_pin_->digital_write(false);
    } else if (load_status_==true){ //testing
      energy_diverted_++;
    }
    
  }
}


}  // namespace ade7953
}  // namespace esphome
