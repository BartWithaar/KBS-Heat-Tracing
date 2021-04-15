#ifndef LoraSend
#define LoraSend

//Defines for pins
#define SCK     5    // GPIO5  -- SX1278's SCK

#define MISO    19   // GPIO19 -- SX1278's MISnO

#define MOSI    27   // GPIO27 -- SX1278's MOSI

#define SS      18   // GPIO18 -- SX1278's CS

#define RST     14   // GPIO14 -- SX1278's RESET

#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define BAND  868E6

//Function prototypes
void LoRaSendInit(void);
void LoRaSendPackage(String device_ID, double temp);

#endif