#pragma once

#include <Arduino.h>
#include <esp32_utilities.h>
#include <system_on_module/settings/soc_pinout.h>
#include <system_on_module/settings/soc_clk_freq.h>

#include <ACAN2517FD.h>
#include <Adafruit_NeoPixel.h>

extern SystemOnChip esp;
extern Terminal terminal;
extern EMMC_Memory emmc;

extern RealTimeClock rtc;
extern SX1509 io_expansion;
extern AD7689 adc;
extern ACAN2517FD can;
extern Adafruit_NeoPixel pixels;

class SystemOnModule
{
public:
    SystemOnModule()
    {
        debugging_enabled = false;
    }

    DateTime system_time;

    void onBootError();

    void initAll(bool debug_enabled);

    bool initIOExpansion();
    bool initRTC();
    bool initADC();
    bool initCAN();
    bool initEMMC();
    void initLED();

protected:
    bool debugging_enabled;
};

extern SystemOnModule SoM;