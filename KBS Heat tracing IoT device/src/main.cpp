#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>

#define LED 13
#define ADCPIN 2
#define NUMSAMPLES 5
int samples[NUMSAMPLES];
int adcData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop() {

  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(ADCPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;

  Serial.print("Average analog reading "); 
  Serial.println(average);

  //adcData = analogRead(ADCPIN);
  //Serial.println(adcData);
  // put your main code here, to run repeatedly:
  digitalWrite(LED, HIGH);
  //Serial.println("LED is on");
  delay(1000);
  digitalWrite(LED, LOW);
  //Serial.println("LED is off");
  delay(1000);
}