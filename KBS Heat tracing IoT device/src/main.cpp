/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Arduino.h>

#define S_TO_uS_FACTOR 1000000
#define TIME_TO_SLEEP  300
#define ADCPIN 2
#define NUMSAMPLES 5
#define CONVERTO_TO_DEGREES(input) (float((input-240)/1.23))

// Initializing functions for later use
void do_send(osjob_t* j);
void low_power_deep_sleep_timer(uint64_t time_in_us);
void setData(int16_t inputData);
float calculateTemperature();
void turnOffRTC();

// Setting up globale variables
int samples[NUMSAMPLES];
float average;
uint8_t sensorOne;
uint8_t sensorTwo;
uint8_t sensorThree;
static uint8_t sensorData[] = {'T', ':', ' ', '0', '0', '0'};

// Saves the LMIC structure during DeepSleep
RTC_DATA_ATTR lmic_t RTC_LMIC;
bool GOTO_DEEPSLEEP = false;

//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] =  { 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xAC, 0x59, 0x00 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] =  { 0xE0, 0xAF, 0x1B, 0xFE, 0xFF, 0xB2, 0x67, 0xAC };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0x95, 0x97, 0x1a, 0x37, 0xb8, 0x3a, 0x4d, 0x83, 0xbe, 0xb8, 0x6b, 0xf9, 0x56, 0x22, 0x16, 0x50 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 18,                 //18
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,              //14
    .dio = {26, 33, 32},    //26,33,32
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            GOTO_DEEPSLEEP = true;
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

float calculateTemperature()
{
  average = 0;

  for (int i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(ADCPIN);
  }
  
  average /= NUMSAMPLES;
  
  return(CONVERTO_TO_DEGREES(average));
}

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

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {       
        setData(calculateTemperature());
        static uint8_t mydata[7] = {sensorData[0],sensorData[1],sensorData[2],sensorData[3],sensorData[4],sensorData[5]} ;
        Serial.printf("%s ",mydata);
        // Serial.println(F(""));
        // Prepare upstream data transmission at the next possible time.      
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void SaveLMICToRTC(int deepsleep_sec)
{
    Serial.println(F("Save LMIC to RTC"));
    RTC_LMIC = LMIC;

    // ESP32 can't track millis during DeepSleep and no option to advanced millis after DeepSleep.
    // Therefore reset DutyCyles

    unsigned long now = millis();

    // EU Like Bands
  #if defined(CFG_LMIC_EU_like)
    Serial.println(F("Reset CFG_LMIC_EU_like band avail"));
    for(int i = 0; i < MAX_BANDS; i++) {
        ostime_t correctedAvail = RTC_LMIC.bands[i].avail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
        if(correctedAvail < 0) {
            correctedAvail = 0;
        }
        RTC_LMIC.bands[i].avail = correctedAvail;
    }

    RTC_LMIC.globalDutyAvail = RTC_LMIC.globalDutyAvail - ((now/1000.0 + deepsleep_sec ) * OSTICKS_PER_SEC);
    if(RTC_LMIC.globalDutyAvail < 0) 
    {
        RTC_LMIC.globalDutyAvail = 0;
    }
  #else
    Serial.println(F("No DutyCycle recalculation function!"));
  #endif
}

void LoadLMICFromRTC()
{
    Serial.println(F("Load LMIC from RTC"));
    LMIC = RTC_LMIC;
}

void PrintRuntime()
{
    long seconds = millis() / 1000;
    Serial.print("Runtime: ");
    Serial.print(seconds);
    Serial.println(" seconds");
}

void low_power_deep_sleep_timer(uint64_t time_in_us){
  Serial.println(F("Go DeepSleep"));
  Serial.flush();
  turnOffRTC();
  esp_sleep_enable_timer_wakeup(time_in_us);
  esp_deep_sleep_start();
}

void turnOffRTC(){
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting"));

    if (RTC_LMIC.seqnoUp != 0)
    {
        LoadLMICFromRTC();
    }
    
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
    static unsigned long lastPrintTime = 0;

    if (!os_queryTimeCriticalJobs(ms2osticksRound((TX_INTERVAL * 1000))) && GOTO_DEEPSLEEP == true)
    {
        SaveLMICToRTC(TX_INTERVAL);
        low_power_deep_sleep_timer(TIME_TO_SLEEP * S_TO_uS_FACTOR);
    }
    else if (lastPrintTime + 2000 < millis())
    {
        Serial.println(F("Cannot sleep"));
        PrintRuntime();
        lastPrintTime = millis();
    }
}
