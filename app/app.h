#pragma once
/*
 * File Name: app.h
 * Project: Mateo's personal project
 * Compartible Hardware: REV1.0
 */

//*********************************************************     READ ME    **********************************************************/

//*****************************************************        LIBRARIES        *****************************************************/
#include <Arduino.h>                // 1. Arduino Framework
#include <esp32_utilities.h>        // 2. ESP32 Utilities :)
#include "../device/soc_pinout.h"   // 3. App specific
#include "../device/soc_settings.h" // 4. App specific

//*****************************************************       DATA TYPES        *****************************************************/

class ApplicationSettings
{
public:
    // TODO: Implement settings
    // TODO: Implement settings from & to JSON
};

class ApplicationRTOS_Objects
{
public:
    // * Terminal
    QueueHandle_t terminal_message_queue = NULL;
    SemaphoreHandle_t terminal_message_queue_mutex = NULL;
    uint16_t terminal_message_queue_length = 50;

    QueueHandle_t terminal_file_queue = NULL;
    SemaphoreHandle_t terminal_file_queue_mutex = NULL;
    uint16_t terminal_file_queue_length = 50;
};

//*************************************************           INTERRUPTS           **************************************************/
//* All interrupts of the app are defined here. The declaration of the interrupt is done inside each method

//*************************************************       TASKS DECLARATION        **************************************************/
void OR21DataLogger(void *parameters);
void OR21TransmissionController(void *parameters);

//*********************************************************       APP       *********************************************************/
class Application
{
public:
    ApplicationSettings settings;

    ApplicationRTOS_Objects rtos;

    //* Systems used in this app
    SystemOnChip esp;
    Terminal terminal;
    EMMC_Memory emmcMemory;
    SPIFFS_Memory spiffsMemory;
    BluetoothLowEnergyServer bleServer;

    // MCP2518FD canController;
    SX1509 ioExpansion;
    // TODO:

    //* Initialize RTOS objects needed by tasks
    ESP_ERROR setupRTOS(); // Defined in "tasks/rtos.h"

    //* App begin
    void begin();
};

//*************************************************       TASKS DECLARATION        **************************************************/

extern Application app; // This is your main app. Initiate in "main.cpp"

// End.