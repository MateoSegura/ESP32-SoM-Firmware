#pragma once

#include "som/som.h"
#include "logger/logger.h"


//******** Data Logger settings
#define NUMBER_OF_CHANNELS 8

#define UART0_BAUD_RATE 115200
#define MICROS_TIMESTAMP_ENABLED false
#define SYSTEM_TIME_ENABLED true
#define DEBUGGING_ENABLED true

// ************* helpers
class Application
{
private:
    DataLogger data_logger;

public:
    void begin();

    ESP_ERROR initGPS();
};

extern Application app;