#pragma once

#include "app/utils.h"
#include "app/som/som.h"
#include <MovingAverageFilter.h>

struct DataLoggerSettings
{
    enum Frequency
    {
        _50_Hz,
        _100_Hz,
        _250_Hz,
        _350_Hz,
        _500_Hz,
    };

    Frequency frequency;
};

struct DataLoggerChannels
{
    DateTime time_stamp; // 27

    double distance;    // 11 (2)
    DateTime lap_time;  // 16
    uint8_t lap_number; // 4

    uint8_t satelites_used; // 2
    double latitude;        // 10 (6)
    double longitude;       // 10 (6)
    double speed;           // 5 (2)
    double heading;         // /?

    double analog_channels[8]; // 48 ( 8 x 6)
    uint16_t raw_analog_channels[8];

    double accel_x;   // 6
    double accel_y;   // 6
    double accel_z;   // 6
    double yaw_rate;  // 6
    double altittude; // 6

    // External Inputs from CAN
    double battery_voltage;   // 4
    double v3v3_rail_voltage; // 4
    double v5v_rail_voltage;  // 4
};

class DataLogger
{
public:
    ESP_ERROR begin(DataLoggerSettings logger_settings);

private:
    struct DataLoggerFile
    {
        char *name;
        DateTime time;
        uint32_t size;
    };

    //*** Variables
    DataLoggerSettings settings;
    DataLoggerFile csv_file;

    //*** Methods
    ESP_ERROR createCSV(DataLoggerFile &file);
    ESP_ERROR toCSVLine(DataLoggerChannels &channels);
};
