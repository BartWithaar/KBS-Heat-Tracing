#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>

#define LED 13
#define ADCPIN 2
#define NUMSAMPLES 5
#define BCOEFFICIENT 3950
#define TEMPERATURENOMINAL 25
#define CONVERTO_TO_DEGREES(input) (float((input-240)/1.23))

int samples[NUMSAMPLES];
int adcData;

static uint8_t sensorData[] = {'T', ':', ' ', '0', '0', '0'};
uint8_t sensorOne;
uint8_t sensorTwo;
uint8_t sensorThree;

void setData(int16_t inputData)
{
  sensorOne = 0;
  sensorTwo = 0;
  sensorThree = 0;

  uint16_t sensorOneFactor;
  uint8_t sensorTwoFactor;

  if(inputData < 0)
  {
    sensorData[2] = '-';
    inputData = inputData*-1;
  }

  for(int i = 99; (i<inputData); i+=100)
  {
    sensorOne++;
  }
  sensorOneFactor = sensorOne*100;

  for(int i = 9; i<(inputData-sensorOneFactor); i+=10)
  {
    sensorTwo++;
  }
  sensorTwoFactor = sensorTwo*10;

  sensorThree = inputData-sensorTwoFactor-sensorOneFactor;

  sensorData[3]= sensorOne + '0';
  sensorData[4]= sensorTwo + '0';
  sensorData[5]= sensorThree + '0';
}

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
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
     
  }
  average /= NUMSAMPLES;

  Serial.print("Average analog reading "); 
  Serial.println(average);
  Serial.println(CONVERTO_TO_DEGREES(average));

  Serial.print("Temp: ");
  //average = (1000*average)/((4095-average)-1);
  setData(CONVERTO_TO_DEGREES(average));
  Serial.println();

  Serial.print("Temp: ");
  //average = (1000*average)/((4095-average)-1);
  setData(-100);
  Serial.println();
  
 
  


  //adcData = analogRead(ADCPIN);
  //Serial.println(adcData);
  // put your main code here, to run repeatedly:
  //digitalWrite(LED, HIGH);
  //Serial.println("LED is on");
  delay(1000);
  //digitalWrite(LED, LOW);
  //Serial.println("LED is off");
  delay(1000);
}



