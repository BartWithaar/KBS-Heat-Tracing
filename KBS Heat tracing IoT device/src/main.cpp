#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <HardwareSerial.h>

#include "TTGO_Sleep.h"

void print_wakeup_reason();

#define S_TO_uS_FACTOR 1000000
#define TIME_TO_SLEEP  5

RTC_DATA_ATTR int bootCount = 0;

#define LED 2

TTGO_Sleep TTGO_sleep = TTGO_Sleep();

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  bootCount = 0;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason();

	Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
	//Go to sleep now
  TTGO_sleep.low_power_deep_sleep_timer(TIME_TO_SLEEP * S_TO_uS_FACTOR);
}

void loop()
{
  digitalWrite(LED, HIGH);
  Serial.println("LED is on");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("LED is off");
  delay(1000);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("Wakeup was caused by the time");
  }
  else
  {
    Serial.println("Wakeup, not caused by timer");
  }
}
