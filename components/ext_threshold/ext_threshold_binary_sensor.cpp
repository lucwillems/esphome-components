#include "ext_threshold_binary_sensor.h"
#include "esphome/core/log.h"
#include "esphome.h"

namespace esphome {
namespace ext_threshold {

static const char *const TAG = "ext_threshold";

void ExtThresholdBinarySensor::setup() {
     uint32_t now=millis();
     this->armed_=true;
     this->treshold_state_=false;
     this->prev_treshold_state_=false;
     this->treshold_state_up_time_=now;
     this->treshold_state_down_time_=now;
     this->prev_delta_value_=0.0;
     this->publish_initial_state(false);
     this->counter_sensor_->publish_state(0.0);
     this->amplitude_sensor_->publish_state(0.0);
     this->time_sensor_->publish_state(0.0);
     publish_adc(0.0);

     // where is the light
     for (EntityBase *obj : App.get_lights()) {
                    ESP_LOGI(TAG, "%12d| light : %s : %s",now,get_object_id().c_str(),obj->get_name().c_str());
     }
     ESP_LOGI(TAG, "init, min_on=%d  max_on=%d, min_off=%d",this->pulse_minimum_time_,this->pulse_maximum_time_,this->pulse_off_time_);
}

void ExtThresholdBinarySensor::set_analog_sensor(sensor::Sensor *analog_sensor) {
  this->analog_sensor_ = analog_sensor;
  this->analog_sensor_->add_on_state_callback([this](float sensor_value) {
       this->on_sensor_value(sensor_value);
       this->prev_treshold_state_=this->treshold_state_;
       this->prev_state_=this->state_;

});
}

void ExtThresholdBinarySensor::start() {
    uint32_t now=millis();
    ESP_LOGI(TAG, "%12d| started",now);
    this->started_=true;
    //initialize calibration
    this->reset_calibration();

}

void ExtThresholdBinarySensor::reset_calibration() {
    uint32_t now=millis();
    this->calibration_value_=0.0;
    this->calibration_cnt_=0;
    this->calibration_start_time_=now;
    this->calibrated_=false;
    ESP_LOGI(TAG, "%12d| calibration started:  time=%d , delta=%d",now,this->calibration_start_time_,this->calibration_time_);

}

void ExtThresholdBinarySensor::pulse(uint32_t now,uint32_t delta_time) {
     this->accumulate_++;
     ESP_LOGI(TAG, "%12d| pulse:  delta=%d , counter=%d , max amplitude=%0.1f",now,delta_time,this->accumulate_,this->max_amplitude_);
     this->time_sensor_->publish_state(delta_time);
     this->counter_sensor_->publish_state(this->accumulate_);
     this->amplitude_sensor_->publish_state(this->max_amplitude_);
     this->calibration_sensor_->publish_state(this->base_value_);
     this->max_amplitude_=0.0;
}


void ExtThresholdBinarySensor::feedback(uint32_t now,float delta_value,float value) {
    if (delta_value < this->upper_threshold_ && delta_value > (this->lower_threshold_/10)) {
        float procent=min(100.0,(delta_value*100.0/this->upper_threshold_));
        //only if feedback is enabled
        if (this->adc_sensor_ != nullptr && this->feedback_enabled_) {
            this->adc_sensor_->publish_state(value);
            this->amplitude_sensor_->publish_state(delta_value);
        }
    }
}

void ExtThresholdBinarySensor::publish_adc(float value) {
    if (this->adc_sensor_ != nullptr) {
            this->adc_sensor_->publish_state(value);
    }
}

void ExtThresholdBinarySensor::on_sensor_value(float value) {
    cnt_++;
    uint32_t now=millis();

    //skip any invalid value
    if (std::isnan(value)) {
        return;
    }

    if (!this->started_) {
        return;
    }

    float delta_value;
    if (!this->absolute_measurement_) {
        //in diff mode we first need to establish base line
        if (!this->calibrated_) {
            this->calibration_value_=this->calibration_value_+value;
            this->calibration_cnt_++;
            if (now >= (this->calibration_start_time_ + this->calibration_time_)) {
                ESP_LOGI(TAG, "%12d| calibration finished, value=%0.2f cnt=%d , start=%d  time=%d",now,this->calibration_value_,this->calibration_cnt_, this->calibration_start_time_,this->calibration_time_);
                this->base_value_=this->calibration_value_/this->calibration_cnt_;
                this->calibrated_=true;
                if (!this->initial_calibrated_) {
                    ESP_LOGI(TAG, "%12d| initial_calibrated, value=%0.2f",now,this->base_value_);
                    this->calibration_sensor_->publish_state(this->base_value_);
                    this->initial_calibrated_=true;
                    return;
                } else {
                    //repeated/retriggerd calibration
                    ESP_LOGI(TAG, "%12d| calibrated, value=%0.2f",now,this->base_value_);
                    this->calibration_sensor_->publish_state(this->base_value_);
                }
            }
        }
         //as long initial calibration is not finished
        if (! this->initial_calibrated_) {
            return;
        }
        //delta measurements
        delta_value=abs(base_value_ - value);
    } else {
        //absolute measurement
        delta_value=value;
    }


    //threshold state change ?
    if (!this->prev_treshold_state_  && delta_value >= this->upper_threshold_)  {
            this->treshold_state_up_time_=now;
            this->treshold_state_=true;
            this->max_amplitude_=value;
            uint32_t delta_time=now - this->treshold_state_down_time_;
            ESP_LOGD(TAG, "%12d| up detected, value=%4.1f delta_value=%4.1f , delta time=%d  armed=%d state=%d",now,value,delta_value,delta_time,this->armed_,this->state_);
            return;
    } else if (this->prev_treshold_state_  && delta_value <= this->lower_threshold_) {
            this->treshold_state_down_time_=now;
            this->treshold_state_=false;
            uint32_t delta_time= now - this->treshold_state_up_time_;
            ESP_LOGD(TAG, "%12d| down detected, value=%4.1f delta_value=%4.1f, delta_time=%d armed=%d state=%d",now,value,delta_value,delta_time,this->armed_,this->state_);
            return;
    } else {
        if (this->prev_delta_value_ != delta_value && delta_value > 10) {
            ESP_LOGD(TAG, "%12d| delta change detected, value=%4.1f delta_value=%4.1f, armed=%d threshold_state=%d state=%d",now,value,delta_value,this->armed_,this->treshold_state_,this->state_);
            this->feedback(now,delta_value,value);
            prev_delta_value_=delta_value;
        }
    }

    //keep track of max amplitude
    if (this->treshold_state_ && delta_value > this->max_amplitude_) {
        this->max_amplitude_=delta_value;
    }

    boolean on_puls=false;
    uint32_t delta_time=0;

    //check state relative to threshold on/off time
    if (this->treshold_state_ && !this->state_  && this->armed_ && now >= (this->treshold_state_up_time_ + this->pulse_minimum_time_)) {
         this->state_=true;
         this->armed_=false;
         this->state_up_time_=now;
         ESP_LOGI(TAG, "%12d| UP STATE accepted value=%4.1f delta_value=%4.1f",now,value,delta_value);
         this->publish_state(true);
         publish_adc(value);
         return;
    }
    else if (this->treshold_state_ && this->state_ && !this->armed_  && now >= (this->state_up_time_ + this->pulse_maximum_time_)) {
         this->state_=false;
         this->armed_=false;
         this->state_down_time_=now;
         uint32_t delta_time=now - this->state_up_time_;
         ESP_LOGI(TAG, "%12d| UP STATE timeout  value=%4.1f delta_value=%4.1f , delta_time=%d",now,value,delta_value,delta_time);
         pulse(now,delta_time);
         this->publish_state(false);
         publish_adc(value);
         return;
    }
    else if (!this->treshold_state_ && !this->armed_ && now >= (this->treshold_state_down_time_ + this->pulse_off_time_)) {
         if (this->state_)  {
                this->state_=false;
                this->state_down_time_=now;
                uint32_t delta_time=now - this->state_up_time_;
                ESP_LOGI(TAG, "%12d| DOWN STATE accepted value=%4.1f delta_value=%4.1f, delta_time=%d",now,value,delta_value,delta_time);
                pulse(now,delta_time);
                publish_adc(value);
                this->publish_state(false);
         } else {
            uint32_t delta_time=now - this->state_down_time_;
            ESP_LOGI(TAG, "%12d| rearming STATE accepted value=%4.1f delta_value=%4.1f, delta_time=%d",now,value,delta_value,delta_time);
         }
         this->armed_=true;
         return;
    }

    //some internal stuff
    if (cnt_ % 1000 == 0) {
        delta_time=now-lastTime;
        ESP_LOGD(TAG, "%12d| loop 1000 %d msec , value=%4.1f delta_value=%4.1f , base=%0.2f armed=%d threshold_state=%d state=%d",now,delta_time,value,delta_value,this->base_value_,this->armed_,this->treshold_state_,this->state_);
        publish_adc(value);
        lastTime=now;
    }
}

void ExtThresholdBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Extend Threshold Binary Sensor", this);
  LOG_SENSOR("  ", "Sensor", this->analog_sensor_);
  ESP_LOGCONFIG(TAG, "  internal: %d", this->internal_);
  ESP_LOGCONFIG(TAG, "  absolute measurements: %d", this->absolute_measurement_);
  ESP_LOGCONFIG(TAG, "  Upper threshold: %.1f", this->upper_threshold_);
  ESP_LOGCONFIG(TAG, "  Lower threshold: %.1f", this->lower_threshold_);
  ESP_LOGCONFIG(TAG, "  calibration time: %d msec", this->calibration_time_);
  ESP_LOGCONFIG(TAG, "  Minimum on time: %d msec", this->pulse_minimum_time_);
  ESP_LOGCONFIG(TAG, "  Minimum off time: %d msec", this->pulse_off_time_);
  ESP_LOGCONFIG(TAG, "  Maximum on time: %d msec", this->pulse_maximum_time_);
}

}  // namespace ext_threshold
}  // namespace esphome
