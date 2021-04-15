#ifndef TTGO_SLEEP_H_
#define TTGO_SLEEP_H_

// Includes
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_bt.h>
// Defines

/**
 * 
 * @author Simon Balk
 */

class TTGO_Sleep{
    public:
    TTGO_Sleep();
    void low_power_deep_sleep_timer(uint64_t time_in_us);
    private:
    void turnOffWifi();
    void turnOffBluetooth();
    void turnOffRTC();
};
extern TTGO_Sleep TTGO_sleep;

#endif