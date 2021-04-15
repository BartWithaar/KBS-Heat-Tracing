#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <LoRaSend.h>

#define LED 2

double Temp;
String device_ID;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  LoRaSendInit();

  device_ID = "Test";
  Temp = 420.69;
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED, HIGH);
  Serial.println("LED is on");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("LED is off");
  delay(1000);

  LoRaSendPackage(device_ID, Temp);
  
}

  
