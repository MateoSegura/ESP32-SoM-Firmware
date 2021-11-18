#pragma once

#include "../som.h"

//******** Data Logger settings
#define NUMBER_OF_CHANNELS 8

#define UART0_BAUD_RATE 115200
#define MICROS_TIMESTAMP_ENABLED false
#define SYSTEM_TIME_ENABLED true
#define DEBUGGING_ENABLED true

void handleError(String debug_message, String system = "   ");
void printMessage(String message, String system = "   ");

class Application
{
public:
    void begin();
};

extern Application app;