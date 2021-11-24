#pragma once

#include "som/som.h"
#include "logger/logger.h"
#include <MPU9250.h>

//******************************************************      Data Logger Settings
#define NUMBER_OF_CHANNELS 8
#define UART0_BAUD_RATE 115200
#define MICROS_TIMESTAMP_ENABLED false
#define SYSTEM_TIME_ENABLED true
#define DEBUGGING_ENABLED true

extern MPU9250 mpu;

//******************************************************      Application Class
class Application
{
private:
public:
    //***** RTOS Stuff
    QueueHandle_t debug_message_queue = NULL;
    SemaphoreHandle_t debug_message_queue_mutex = NULL;
    uint16_t debug_message_queue_length = 20;

    void begin();

    ESP_ERROR initRTOS();
    ESP_ERROR terminalRTOS();

    ESP_ERROR initGPS();
};

extern Application app;
