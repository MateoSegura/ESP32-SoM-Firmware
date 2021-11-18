#include "app.h"

const char file_name[] = "/test.csv";

SystemOnModule SoM;

void runWriteTest();
void dataLogger(void *parameters);

class LoggerData
{
public:
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

    double accel_x;   // 6
    double accel_y;   // 6
    double accel_z;   // 6
    double yaw_rate;  // 6
    double altittude; // 6

    // External Inputs from CAN
    double battery_voltage;   // 4
    double v3v3_rail_voltage; // 4
    double v5v_rail_voltage;  // 4

    void toCSVLine();
};

void LoggerData::toCSVLine()
{
    long initial_time = micros();
    char data[512];
    uint16_t length = 0;

    time_stamp = SoM.system_time;

    //* 1. Time stamp (27)
    char timestamp_str[27];
    length += sprintf(timestamp_str, "%4i-%02i-%02iT%02i:%02i:%02i.%03i%03iZ,",
                      time_stamp.year,
                      time_stamp.month,
                      time_stamp.day,
                      time_stamp.hours,
                      time_stamp.minutes,
                      time_stamp.seconds,
                      time_stamp.milliseconds,
                      time_stamp.microseconds);

    strcpy(data, timestamp_str);

    char analog_ch_str[(6 * NUMBER_OF_CHANNELS) + NUMBER_OF_CHANNELS];
    length += sprintf(analog_ch_str, "%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf",
                      analog_channels[0],
                      analog_channels[1],
                      analog_channels[2],
                      analog_channels[3],
                      analog_channels[4],
                      analog_channels[5],
                      analog_channels[6],
                      analog_channels[7]);

    strcat(data, analog_ch_str);
    strcat(data, "\n");

    for (int i = 0; i < 4; i++)
    {
        strcat(data, timestamp_str);
        strcat(data, analog_ch_str);
        strcat(data, "\n");
    }

    strcat(data, timestamp_str);
    strcat(data, analog_ch_str);

    terminal.printMessage(TerminalMessage(String(length), "MMC", INFO, micros(),
                                          micros() - initial_time));

    initial_time = micros();

    emmc.appendFile(file_name, data);

    terminal.printMessage(TerminalMessage("File saved", "MMC", INFO, micros(),
                                          micros() - initial_time));

    //* 2. distance;    // 11 (2)
    //* 3. lap_time;  // 16
    //* 4. lap_number; // 4

    //* 5. satelites_used; // 2
    //* 6.latitude;        // 10 (6)
    //* 7. longitude;       // 10 (6)
    //* 8.speed;           // 5 (2)
    //* 9. heading;         // /?

    //* 10.analog_channels[8]; // 48 ( 8 x 6)

    //* 11. accel_x;   // 6
    //* 12. accel_y;   // 6
    //* 13. accel_z;   // 6
    //* 14. yaw_rate;  // 6
    //* 15. altittude; // 6

    //* 16. battery_voltage;   // 4
    //* 17. v3v3_rail_voltage; // 4
    //* 18. v5v_rail_voltage;  // 4
}

void Application::begin()
{
    long initial_time;

    // 1. Begin Serial port for debugging
    esp.uart0.begin(UART0_BAUD_RATE);
    terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);
    terminal.printBanner("Data Logging Application");

    // 2. Init System on Module
    SoM.initAll(DEBUGGING_ENABLED);

    // 3. Create file
    char data[] = "1,2,3,4,5,6,7,8,9\n";

    initial_time = micros();
    ESP_ERROR create_file = emmc.writeFile(file_name, data, 36);

    if (create_file.on_error)
        handleError(create_file.debug_message, "MMC");

    terminal.printMessage(TerminalMessage("File created", "MMC", INFO, micros(),
                                          micros() - initial_time));

    xTaskCreatePinnedToCore(dataLogger, "Data Logger", 5000, nullptr, 1, nullptr, 1);
}

#define BLOCKS_TO_WRITE 1000 // Blocks of 512 bytes

void runWriteTest()
{
    // 3. Attempt to write
    long initial_time = micros();
    double write_times_sum = 0;

    char data[] = "1,2,3,4,5,6,7,8";

    for (int i = 0; i < (BLOCKS_TO_WRITE - 1); i++)
    {
        long initial_write_time = micros();
        emmc.appendFile(file_name, data);

        write_times_sum += micros() - initial_write_time;
    }

    // 4. Display stats to user
    terminal.printMessage(TerminalMessage("Writing test done: ", "MMC", INFO, micros(), micros() - initial_time));
    esp.uart0.println();

    double avg_write_time = write_times_sum / (BLOCKS_TO_WRITE * 1000);
    terminal.println("Average write time: " + String(avg_write_time, 2) + "mS");

    long bytes_written = 512 * BLOCKS_TO_WRITE;
    terminal.println("Number of bytes written: " + String(bytes_written) + " bytes");

    double avg_write_speed = (512 * BLOCKS_TO_WRITE) / avg_write_time;
    terminal.println("Average write speed: " + String((avg_write_speed / 1000000), 2) + " MB/s");

    esp.uart0.println();
}

double toVolts(uint16_t raw_reading)
{
    double result = (4.096 * raw_reading) / 65536;
    return result;
}

void dataLogger(void *parameters)
{
    while (1)
    {
        yield();

        long initial_time;

        // 1. Sample all 8 analog channels
        uint16_t raw_readings[NUMBER_OF_CHANNELS];
        uint16_t temp_reading;

        initial_time = micros();
        adc.readChannels(NUMBER_OF_CHANNELS, UNIPOLAR_MODE, raw_readings, &temp_reading);

        terminal.printMessage(TerminalMessage("ADC sampled", "ADC", INFO, micros(),
                                              micros() - initial_time));

        uint16_t sample_size = (5 * NUMBER_OF_CHANNELS) + NUMBER_OF_CHANNELS + 1;
        int i = 0;

        char data[sample_size];

        LoggerData temp;

        for (int i = 0; i < NUMBER_OF_CHANNELS; i++)
            temp.analog_channels[i] = toVolts(raw_readings[i]);

        temp.toCSVLine();

        // for (int i = 0; i < NUMBER_OF_CHANNELS; i++)
        // {
        //     char buffer[5];
        //     dtostrf(raw_readings[i], 5, 0, buffer);
        //     buffer[4] = ',';
        //     strcat(data, buffer);
        // }

        // 2. Save to uSD card
        // initial_time = micros();
    }

    // 2. Save to sd card
}

// ************* helpers
void handleError(String debug_message, String system)
{
    terminal.printMessage(TerminalMessage(debug_message, system, ERROR, micros()));

    pixels.setPixelColor(0, pixels.Color(50, 0, 0));
    pixels.show();

    esp.uart0.print("\n\nSystem Aborted. Restart\n\n");
    while (1)
        ;
}

void printMessage(String message, String system)
{
    terminal.printMessage(TerminalMessage(message, system, INFO, micros()));
}