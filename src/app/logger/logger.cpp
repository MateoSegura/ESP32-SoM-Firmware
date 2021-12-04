#include "../app.h"

bool debugging = true;

//****** Global data
DataLoggerChannels global_data;

//****** Filters
MovingAverageFilter movingAverageFilter(20);

//****** Loggers
void simpleLogger();

ESP_ERROR DataLogger::begin(DataLoggerSettings logger_settings)
{
    ESP_ERROR err;
    TerminalMessage message;

    //* 1. Register RTOS
    sample_imu_semaphore = xSemaphoreCreateBinary();
    data_samples_queue = xQueueCreate(50, sizeof(DataLoggerChannels));

    //* 1. Create file name
    char data_file_name[] = "/test.csv";

    csv_file.name = data_file_name;
    csv_file.time = SoM.system_time;

    //* 2. Create .csv file in memory
    ESP_ERROR create_file = createCSV(csv_file);

    if (create_file.on_error)
        handleError(create_file.debug_message, "LOG");

    message = TerminalMessage("File created");

    printMessage(message);

    //* 3. Start simple logger
    simpleLogger();

    return err;
}

//***********************************************************************************
//********************************* Logger Tasks ************************************
//***********************************************************************************

//* Analog to Digital Converter
void sampleADC(void *parameters)
{
    TerminalMessage adc_debug_message;

    while (1)
    {
        uint16_t raw_readings[NUM_ANALOG_INPUTS];
        uint16_t temp_reading;

        adc.readChannels(8, UNIPOLAR_MODE, raw_readings, &temp_reading);

        for (int i = 0; i < NUMBER_OF_CHANNELS; i++)
            global_data.analog_channels[i] = raw_readings[i];

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

//* GPS
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
}

//* Internal Motion Unit
void IRAM_ATTR imuInterrupt()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(logger.sample_imu_semaphore, &xHigherPriorityTaskWoken);
}

void sampleIMU(void *parameters)
{
    TerminalMessage message;

    while (1)
    {
        xSemaphoreTake(logger.sample_imu_semaphore, portMAX_DELAY);

        long initial_time = micros();

        if (imu.update())
        {
            global_data.accel_x = imu.getAccX();
            global_data.accel_y = imu.getAccY();
            global_data.accel_z = imu.getAccZ();
        }
    }
}

//* Save data
void addSampleToQueue(void *parameters)
{
    DataLoggerChannels data_array[50];
    uint8_t index = 0;
    TerminalMessage message;

    while (1)
    {
        data_array[index] = global_data;
        xQueueSend(logger.data_samples_queue, (void *)&data_array[index], 0);

        message.body = data_array[index].accel_x;
        message.body += " ";
        message.body += data_array[index].analog_channels[0];
        printMessage(message);

        index++;

        if (index == 50)
            index = 0;

        vTaskDelay(5 / portTICK_PERIOD_MS); // 200Hz
    }
}

void saveToMemory(void *parameters)
{
    DataLoggerChannels save_channels[5];
    uint8_t index = 0;

    while (1)
    {
        for (index = 0; index < 5; index++)
            xQueueReceive(logger.data_samples_queue, (void *)&save_channels[index], portMAX_DELAY);
    }
}

//***********************************************************************************
//********************************* Data Loggers ************************************
//***********************************************************************************
void simpleLogger()
{
    pinMode(IMU0_INT, PULLUP);
    attachInterrupt(IMU0_INT, imuInterrupt, CHANGE);

    xTaskCreatePinnedToCore(sampleADC, "Sample ADC", 5000, nullptr, 24, nullptr, 1);
    // xTaskCreatePinnedToCore(sampleGPS, "Sample GPS", 5000, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(sampleIMU, "Sample IMU", 5000, nullptr, 5, nullptr, 1);

    xTaskCreatePinnedToCore(addSampleToQueue, "Sample Logger", 20000, nullptr, 25, nullptr, 0);
}

//***********************************************************************************
//************************************ Methods **************************************
//***********************************************************************************

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
