#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <LoRaSend.h>

  uint8_t counter = 0;

  //Counter for packages
  void LoRaSendInit(void){
  Serial.begin(115200);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  
  //Waiting for serial
  while (!Serial);
    
  Serial.println("LoRa Sender \n");
  
  //Check if lora is started properly
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed! \n");
    while (1);
  }

  LoRa.setSyncWord(0xF3);
  LoRa.setTxPower(20);

}

void LoRaSendPackage(String device_ID, double temp) {
  Serial.print("Sending packet: \n");
  Serial.println(counter);

  //Start package
  LoRa.beginPacket();
  //Send message
  LoRa.print(device_ID);
  LoRa.print(temp);
  //End package
  LoRa.endPacket();

  Serial.println("Package sended");

  counter++;
  delay(5000);
}
