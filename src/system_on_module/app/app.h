#pragma once

#include "../som.h"

#define UART0_BAUD_RATE 115200
#define MICROS_TIMESTAMP_ENABLED false
#define SYSTEM_TIME_ENABLED true
#define DEBUGGING_ENABLED false

SystemOnModule SoM;

class Application
{
public:
    void begin()
    {
        esp.uart0.begin(UART0_BAUD_RATE);
        terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);

        SoM.initAll(DEBUGGING_ENABLED);
    }
};

extern Application app;