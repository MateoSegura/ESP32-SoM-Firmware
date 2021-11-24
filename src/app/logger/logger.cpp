#include "../app.h"

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

int count;

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

        if (mpu.update())
        {
            esp.uart0.print(mpu.getAccX());
            esp.uart0.print(", ");
            esp.uart0.print(mpu.getAccY());
            esp.uart0.print(", ");
            esp.uart0.print(mpu.getAccZ());
            esp.uart0.print("\t");
            esp.uart0.println((micros() - initial_time) / 1000);
        }
    }

    while (1)
    {
        // xSemaphoreTake(logger.sample_imu_semaphore, portMAX_DELAY);

        // if (imu.readByte(0x68, INT_STATUS) & 0x01)
        // {
        //     long initial_time = micros();

        //     imu.delt_t = millis() - imu.count;

        //     imu.readAccelData(imu.accelCount); // Read the x/y/z adc values
        //     imu.readGyroData(imu.gyroCount);   // Read the x/y/z adc values
        //     imu.readMagData(imu.magCount);     // Read the x/y/z adc values
        //     imu.updateTime();

        //     MahonyQuaternionUpdate(imu.ax, imu.ay, imu.az, imu.gx * DEG_TO_RAD,
        //                            imu.gy * DEG_TO_RAD, imu.gz * DEG_TO_RAD, imu.my,
        //                            imu.mx, imu.mz, imu.deltat);

        //     imu.yaw = atan2(2.0f * (*(getQ() + 1) * *(getQ() + 2) + *getQ() * *(getQ() + 3)), *getQ() * *getQ() + *(getQ() + 1) * *(getQ() + 1) - *(getQ() + 2) * *(getQ() + 2) - *(getQ() + 3) * *(getQ() + 3));
        //     imu.pitch = -asin(2.0f * (*(getQ() + 1) * *(getQ() + 3) - *getQ() * *(getQ() + 2)));
        //     imu.roll = atan2(2.0f * (*getQ() * *(getQ() + 1) + *(getQ() + 2) * *(getQ() + 3)), *getQ() * *getQ() - *(getQ() + 1) * *(getQ() + 1) - *(getQ() + 2) * *(getQ() + 2) + *(getQ() + 3) * *(getQ() + 3));
        //     imu.pitch *= RAD_TO_DEG;
        //     imu.yaw *= RAD_TO_DEG;

        //     // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
        //     // 	8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
        //     // - http://www.ngdc.noaa.gov/geomag-web/#declination
        //     imu.yaw -= 8.5;
        //     imu.roll *= RAD_TO_DEG;

        //     esp.uart0.print("ax = ");
        //     esp.uart0.print((int)1000 * imu.ax);
        //     esp.uart0.print(" ay = ");
        //     esp.uart0.print((int)1000 * imu.ay);
        //     esp.uart0.print(" az = ");
        //     esp.uart0.print((int)1000 * imu.az);
        //     esp.uart0.println(" mg");

        //     esp.uart0.print("gx = ");
        //     esp.uart0.print(imu.gx, 2);
        //     esp.uart0.print(" gy = ");
        //     esp.uart0.print(imu.gy, 2);
        //     esp.uart0.print(" gz = ");
        //     esp.uart0.print(imu.gz, 2);
        //     esp.uart0.println(" deg/s");

        //     esp.uart0.print("mx = ");
        //     esp.uart0.print((int)imu.mx);
        //     esp.uart0.print(" my = ");
        //     esp.uart0.print((int)imu.my);
        //     esp.uart0.print(" mz = ");
        //     esp.uart0.print((int)imu.mz);
        //     esp.uart0.println(" mG");

        //     esp.uart0.print("q0 = ");
        //     esp.uart0.print(*getQ());
        //     esp.uart0.print(" qx = ");
        //     esp.uart0.print(*(getQ() + 1));
        //     esp.uart0.print(" qy = ");
        //     esp.uart0.print(*(getQ() + 2));
        //     esp.uart0.print(" qz = ");
        //     esp.uart0.println(*(getQ() + 3));

        //     esp.uart0.print("Yaw, Pitch, Roll: ");
        //     esp.uart0.print(imu.yaw, 2);
        //     esp.uart0.print(", ");
        //     esp.uart0.print(imu.pitch, 2);
        //     esp.uart0.print(", ");
        //     esp.uart0.println(imu.roll, 2);

        //     esp.uart0.print("rate = ");
        //     esp.uart0.print((float)imu.sumCount / imu.sum, 2);
        //     esp.uart0.println(" Hz");

        //     imu.count = millis();
        //     imu.sumCount = 0;
        //     imu.sum = 0;

        // message.body = "accl_x";
        // message.body += String(1000 * imu.ax);
        // message.body += "accl_y";
        // message.body += imu.ay;
        // message.body += "accl_z";
        // message.body += imu.az;

        // message.body += "gyro_x";
        // message.body += imu.gx;
        // message.body += "gyro_y";
        // message.body += imu.gy;
        // message.body += "gyro_z";
        // message.body += imu.gz;

        // message.process_time = micros() - initial_time;

        // printMessage(message);
        //}
    }
}

void simpleLogger(void *parameters)
{
    pinMode(IMU0_INT, PULLUP);
    attachInterrupt(IMU0_INT, imuInterrupt, CHANGE);

    // xTaskCreatePinnedToCore(sampleADC, "Sample ADC", 5000, nullptr, 25, nullptr, 1);
    // xTaskCreatePinnedToCore(sampleGPS, "Sample GPS", 5000, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(sampleIMU, "Sample IMU", 5000, nullptr, 5, nullptr, 1);

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
