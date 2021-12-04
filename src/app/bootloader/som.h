#pragma once

#include <Arduino.h>
#include <esp32_utilities.h>
#include <app/settings/soc_pinout.h>
#include <app/settings/soc_clk_freq.h>

#include <ACAN2517FD.h>
#include <Adafruit_NeoPixel.h>
#include "SparkFun_Ublox_Arduino_Library.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

extern SFE_UBLOX_GPS gps;

extern SystemOnChip esp;
extern Terminal terminal;
extern EMMC_Memory emmc;

extern RealTimeClock rtc;
extern SX1509 io_expansion;
extern AD7689 adc;
extern ACAN2517FD can;
extern MPU9250 imu;
extern Adafruit_NeoPixel pixels;
extern Adafruit_BME680 bme;

class SystemOnModule
{
public:
    SystemOnModule()
    {
        debugging_enabled = false;
    }

    DateTime system_time;

    void initAll(bool debug_enabled);

    bool initIOExpansion();
    bool initRTC();
    bool initADC();
    bool initCAN();
    bool initIMU();
    bool initBME();
    bool initMMC();
    void initLED();

    void initTimers();

protected:
    bool debugging_enabled;

public:
    void bootError();
};

extern SystemOnModule SoM;
