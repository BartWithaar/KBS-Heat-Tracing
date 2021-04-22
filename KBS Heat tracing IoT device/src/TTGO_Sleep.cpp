#include "TTGO_Sleep.h"
#include <LoRa.h>

TTGO_Sleep::TTGO_Sleep(){
    
}

//put the t-beam sleep with the least amount off power consumption as possible
void TTGO_Sleep::low_power_deep_sleep_timer(uint64_t time_in_us){
  //turnOffRTC();
  LoRa.sleep();
  esp_sleep_enable_timer_wakeup(time_in_us);
  esp_deep_sleep_start();
}

//Power-down of RTC peripherals to save battery
void  TTGO_Sleep::turnOffRTC(){
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
}