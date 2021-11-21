#include "logger.h"

bool debugging = true;

//****** Filters
MovingAverageFilter movingAverageFilter(20);

//****** Loggers
void simpleLogger(void *parameters);

ESP_ERROR DataLogger::begin(DataLoggerSettings logger_settings)
{
    ESP_ERROR err;
    TerminalMessage message;

    long initial_time;

    //* 1. Register RTOS
    sample_imu_semaphore = xSemaphoreCreateBinary();

    //* 1. Create file name
    csv_file.name = "/test.csv";
    csv_file.time = SoM.system_time;

    //* 2. Create .csv file in memory
    initial_time = micros();
    ESP_ERROR create_file = createCSV(csv_file);

    if (create_file.on_error)
        handleError(create_file.debug_message, "LOG");

    message = TerminalMessage("File created");

    printMessage(message);

    //* 3. Start simple logger
    xTaskCreatePinnedToCore(simpleLogger, "Simple Log", 10000, nullptr, 1, nullptr, 1);

    return err;
}

// TODO: Figure out how tf im gonna get this shit to work lol

void sampleADC(void *parameters)
{
    TerminalMessage adc_debug_message;

    while (1)
    {
        uint16_t raw_readings[NUM_ANALOG_INPUTS];
        uint16_t temp_reading;

        adc.readChannels(8, UNIPOLAR_MODE, raw_readings, &temp_reading);

        if (debugging)
        {
            // adc_debug_message = TerminalMessage("Sampled ADC");
            // printMessage(adc_debug_message);
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void sampleGPS(void *parameters)
{
    long initial_time;

    DataLoggerChannels channels;
    TerminalMessage message;

    while (1)
    {
        //* 2. Get GPS
        initial_time = micros();
        if (gps.getHNRPVT() == true) // If setAutoHNRPVT was successful and new data is available
        {
            channels.latitude = gps.hnrPVT.lat;
            channels.longitude = gps.hnrPVT.lon;
            channels.satelites_used = gps.hnrPVT.gpsFix;
            channels.speed = gps.hnrPVT.gSpeed;
            channels.heading = gps.hnrPVT.headVeh;

            message = TerminalMessage("Lat: " + String(gps.hnrPVT.lat) +
                                          "\tLong: " + String(gps.hnrPVT.lon) +
                                          "\tFix: " + String(gps.hnrPVT.gpsFix) +
                                          "\tSpeed: " + String(gps.hnrPVT.gSpeed) +
                                          "\tHeading: " + String(gps.hnrPVT.headVeh),
                                      "LOG", INFO, micros(), micros() - initial_time);

            printMessage(message);
        }
    }

    //* 2. Sample IMU
}

void IRAM_ATTR imuInterrupt()
{
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // xSemaphoreGiveFromISR(logger.sample_imu_semaphore, &xHigherPriorityTaskWoken);
}

void sampleIMU(void *parameters)
{
    TerminalMessage message;

    while (1)
    {
        // xSemaphoreTake(logger.sample_imu_semaphore, portMAX_DELAY);

        long initial_time = micros();
        if (!imu.readSensor())
        {
            message.body = "Sampled";
        }
        else
        {
            message.body = "";
        }

        // message.body += imu.getAccelX_mss();
        // message.body += imu.getAccelX_mss();
        // message.body += "\t";
        // message.body += imu.getAccelY_mss();
        // message.body += "\t";
        // message.body += imu.getAccelZ_mss();
        // message.body += "\t";
        // message.body += imu.getGyroX_rads();
        // message.body += "\t";
        // message.body += imu.getGyroY_rads();
        // message.body += "\t";
        // message.body += imu.getGyroZ_rads();
        // message.body += "\t";
        // message.body += imu.getMagX_uT();
        // message.body += "\t";
        // message.body += imu.getMagY_uT();
        // message.body += "\t";
        // message.body += imu.getMagZ_uT();
        // message.body += "\t";
        // message.body += imu.getTemperature_C();

        message.process_time = micros() - initial_time;
        printMessage(message);
    }
}

void simpleLogger(void *parameters)
{
    pinMode(IMU0_INT, PULLUP);
    attachInterrupt(IMU0_INT, imuInterrupt, RISING);

    xTaskCreatePinnedToCore(sampleADC, "Sample ADC", 5000, nullptr, 25, nullptr, 1);
    xTaskCreatePinnedToCore(sampleGPS, "Sample GPS", 5000, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(sampleIMU, "Sample IMU", 5000, nullptr, 3, nullptr, 1);

    vTaskDelete(nullptr);
}

//*************************** METHODS
ESP_ERROR DataLogger::createCSV(DataLoggerFile &file)
{
    ESP_ERROR err;

    // 1. Create header string
    char csv_header_str[512];
    uint16_t str_length = 0;

    str_length += sprintf(csv_header_str,
                          "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
                          "time_stamp",
                          "distance",
                          "lap_time",
                          "lap_number",
                          "satelites_used",
                          "latitude",
                          "longitude"
                          "speed",
                          "heading",
                          "analog_ch0",
                          "analog_ch1",
                          "analog_ch2",
                          "analog_ch3",
                          "analog_ch4",
                          "analog_ch5",
                          "analog_ch6",
                          "analog_ch7",
                          "accel_x",
                          "accel_y",
                          "accel_z",
                          "yaw_rate",
                          "altitude",
                          "battery_volts",
                          "3v3_rail",
                          "5v_rail");

    ESP_ERROR create_file = emmc.writeFile(file.name, csv_header_str, str_length);

    if (create_file.on_error)
    {
        err.on_error = true;
        err.debug_message = create_file.debug_message;
    }

    return err;
}
