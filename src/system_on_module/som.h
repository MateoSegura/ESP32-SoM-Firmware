// #pragma once

// #include <Arduino.h>
// #include <esp32_utilities.h>

// #include "settings/soc_pinout.h"
// #include "settings/soc_clk_freq.h"

// #include "Adafruit_NeoPixel.h"

// //* ESP32
// extern SystemOnChip esp;
// extern Terminal terminal;
// extern SPIFFS_Memory spiffs;
// extern EMMC_Memory emmc;
// // extern BluetoothLowEnergyServer bleServer;

// //* Peripherals
// extern SX1509 ioExpansion;
// extern RealTimeClock rtc;
// extern Adafruit_NeoPixel ledStrip;
// extern AD7689 adc;

// class ESP32_SystemOnModule
// {
// public:
//     // Init all Systems
//     ESP_ERROR begin(bool enable_debugging = true);

//     // Individual systems init
//     ESP_ERROR initIO_Expansion();
//     // ESP_ERROR initRTC();
//     ESP_ERROR initADC();
//     // ESP_ERROR initCAN();
//     //  ESP_ERROR initEMMC();
//     //  ESP_ERROR initIMU();

// private:
//     bool debugging;
//     bool v5v_enabled;
// };

// extern ESP32_SystemOnModule SoM;

// // End.