#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/light/addressable_light.h"

namespace esphome {
namespace ext_threshold {

class ExtThresholdBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void dump_config() override;
  void setup() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_analog_sensor(sensor::Sensor *analog_sensor);
  void set_pulse_time_sensor(sensor::Sensor *time_sensor)               { this->time_sensor_= time_sensor; }
  void set_pulse_amplitude_sensor(sensor::Sensor *amplitude_sensor)     { this->amplitude_sensor_= amplitude_sensor; }
  void set_pulse_counter_sensor(sensor::Sensor *counter_sensor)         { this->counter_sensor_= counter_sensor; }
  void set_calibration_sensor(sensor::Sensor *calibration_sensor)       { this->calibration_sensor_= calibration_sensor; }
  void set_adc_sensor(sensor::Sensor *adc_sensor)                       { this->adc_sensor_= adc_sensor; }

  void set_absolute_measurement(bool absolute) { this->absolute_measurement_=absolute;}
  void set_calibration_time(uint16_t timeout)  { this->calibration_time_=timeout;}
  void reset_calibration();
  void start();

  // enable/disable feedback stream
  bool is_feedback_enabled()  { return this->feedback_enabled_;}
  void set_feedback_enabled(bool enable)  {this->feedback_enabled_=enable;}
  void toggle_feedback_enabled()  {this->feedback_enabled_=!this->feedback_enabled_;}

  void set_upper_threshold(float threshold)     { this->upper_threshold_ = threshold; }
  void set_lower_threshold(float threshold)     { this->lower_threshold_ = threshold; }
  void set_pulse_minimum_time(uint16_t timeout) { this->pulse_minimum_time_ = timeout; }
  void set_pulse_maximum_time(uint16_t timeout) { this->pulse_maximum_time_ = timeout; }
  void set_pulse_off_time(uint16_t timeout)     { this->pulse_off_time_ = timeout; }

  void set_armed(bool armed) {this->armed_ = armed;}

  void on_sensor_value(float value);

 protected:

  sensor::Sensor *analog_sensor_{nullptr};
  sensor::Sensor *counter_sensor_{nullptr};
  sensor::Sensor *time_sensor_{nullptr};
  sensor::Sensor *amplitude_sensor_{nullptr};
  sensor::Sensor *calibration_sensor_{nullptr};
  sensor::Sensor *adc_sensor_{nullptr};


  bool started_=false;
  bool feedback_enabled_=false;

  bool absolute_measurement_;
  float prev_delta_value_;

  //use for auto calibration
  float base_value_=0.0;
  float calibration_value_=0.0;
  uint16_t calibration_cnt_=0;
  bool initial_calibrated_=false;
  bool calibrated_=false;
  uint32_t calibration_start_time_;
  uint32_t calibration_time_=2000;

  void pulse(uint32_t now,uint32_t delta_time);
  void feedback(uint32_t now,float delta_value,float value);

  bool armed_;
  bool treshold_state_;
  bool prev_treshold_state_;
  bool state_;
  bool prev_state_;
  float max_amplitude_;

  uint32_t treshold_state_up_time_;
  uint32_t treshold_state_down_time_;
  uint32_t state_up_time_;
  uint32_t state_down_time_;

  uint32_t pulse_minimum_time_;
  uint32_t pulse_maximum_time_;
  uint32_t pulse_off_time_;

  uint32_t lastTime;
  uint32_t cnt_; //loop counter
  uint32_t accumulate_;


  float upper_threshold_;
  float lower_threshold_;
};

}  // namespace ext_threshold
}  // namespace esphome
