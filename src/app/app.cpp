#include "app.h"
#include "utils/utils.h"

SystemOnModule SoM;
DataLogger logger;
MPU9250 mpu;

void i2cTest();

void Application::begin()
{
    long initial_time;

    // 1. Begin esp.uart0 port for debugging
    esp.uart0.begin(UART0_BAUD_RATE);
    terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);
    terminal.printBanner("Data Logging Application");

    // 2. Init System on Module
    SoM.initAll(DEBUGGING_ENABLED);

    // 3. Register RTOS variables
    initRTOS();

    esp.i2c1.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, I2C1_CLK_FREQUENCY);

    MPU9250Setting setting;
    setting.accel_fs_sel = ACCEL_FS_SEL::A16G;
    setting.gyro_fs_sel = GYRO_FS_SEL::G2000DPS;
    setting.mag_output_bits = MAG_OUTPUT_BITS::M16BITS;
    setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_500HZ;
    setting.gyro_fchoice = 0x03;
    setting.gyro_dlpf_cfg = GYRO_DLPF_CFG::DLPF_41HZ;
    setting.accel_fchoice = 0x01;
    setting.accel_dlpf_cfg = ACCEL_DLPF_CFG::DLPF_45HZ;

    if (!mpu.setup(0x68, setting, esp.i2c1))
    {
        SoM.onBootError();
    }

    while (1)
    {
        i2cTest();
    }

    // 3. Init terminal output tasks for all other tasks
    // xTaskCreatePinnedToCore(terminalOutput, "Terminal", 10000, nullptr, 1, nullptr, 0);

    // 3. Init Carrier Board hardware
    // ESP_ERROR init_gps = initGPS();

    // if (init_gps.on_error)
    //     handleError(init_gps.debug_message, "APP");

    // TerminalMessage message = TerminalMessage("GPS Initialized", "APP", INFO, micros());
    // printMessage(message);

    // 3. Init Logger
    // DataLoggerSettings logger_settings;
    // logger_settings.frequency = DataLoggerSettings::_50_Hz;

    // logger.begin(logger_settings);
}

ESP_ERROR Application::initGPS()
{
    ESP_ERROR err;

    if (!gps.begin(esp.i2c1))
    {
        err.on_error = true;
        err.debug_message = "Could not initialize GPS";
        return err;
    }

    gps.setI2COutput(COM_TYPE_UBX);                 // Set the I2C port to output UBX only (turn off NMEA noise)
    gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR

    if (!gps.setHNRNavigationRate(20)) // Set the High Navigation Rate to 10Hz
    {
        err.on_error = true;
        err.debug_message = "setHNRNavigationRate was NOT successful";
        return err;
    }

    boolean usingAutoHNRAtt = false;
    boolean usingAutoHNRDyn = false;
    boolean usingAutoHNRPVT = false;

    // usingAutoHNRAtt = gps.setAutoHNRAtt(true); // Attempt to enable auto HNR attitude messages
    // usingAutoHNRDyn = gps.setAutoHNRDyn(true); // Attempt to enable auto HNR vehicle dynamics messages
    // usingAutoHNRPVT = gps.setAutoHNRPVT(true); // Attempt to enable auto HNR PVT messages

    gps.setAutoHNRPVT(true);

    return err;
}

//******************************************************      RTOS DECLARATIONS
ESP_ERROR Application::initRTOS()
{
    ESP_ERROR err;

    ESP_ERROR terminal_rtos = terminalRTOS();
    if (terminal_rtos.on_error)
        return terminal_rtos;

    return err;
}

ESP_ERROR Application::terminalRTOS()
{
    ESP_ERROR err;

    app.debug_message_queue = xQueueCreate(app.debug_message_queue_length, sizeof(TerminalMessage)); // Queue
    app.debug_message_queue_mutex = xSemaphoreCreateMutex();                                         // Mutex

    if (app.debug_message_queue == NULL || app.debug_message_queue_mutex == NULL)
    {
        err.on_error = true;
        err.debug_message = "Could not create Terminal messages queue objects.";
    }

    return err;
}

//******************************************************      I2C Test
void i2cTest()
{
    byte error, address;
    int nDevices;

    esp.uart0.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        esp.i2c0.beginTransmission(address);
        error = esp.i2c0.endTransmission();

        if (error == 0)
        {
            esp.uart0.print("I2C0 device found at address 0x");
            if (address < 16)
                esp.uart0.print("0");
            esp.uart0.print(address, HEX);
            esp.uart0.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            esp.uart0.print("Unknown error at address 0x");
            if (address < 16)
                esp.uart0.print("0");
            esp.uart0.println(address, HEX);
        }
    }
    if (nDevices == 0)
        esp.uart0.println("No I2C0 devices found\n");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        esp.i2c1.beginTransmission(address);
        error = esp.i2c1.endTransmission();

        if (error == 0)
        {
            esp.uart0.print("I2C1 device found at address 0x");
            if (address < 16)
                esp.uart0.print("0");
            esp.uart0.print(address, HEX);
            esp.uart0.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            esp.uart0.print("Unknown error at address 0x");
            if (address < 16)
                esp.uart0.print("0");
            esp.uart0.println(address, HEX);
        }
    }
    if (nDevices == 0)
        esp.uart0.println("No I2C1 devices found\n");

    delay(1000); // wait 5 seconds for next scan
}

// void runWriteTest()
// {
//     // 3. Attempt to write
//     long initial_time = micros();
//     double write_times_sum = 0;

//     char data[] = "1,2,3,4,5,6,7,8";

//     for (int i = 0; i < (BLOCKS_TO_WRITE - 1); i++)
//     {
//         long initial_write_time = micros();
//         emmc.appendFile(file_name, data);

//         write_times_sum += micros() - initial_write_time;
//     }

//     // 4. Display stats to user
//     terminal.printMessage(TerminalMessage("Writing test done: ", "MMC", INFO, micros(), micros() - initial_time));
//     esp.uart0.println();

//     double avg_write_time = write_times_sum / (BLOCKS_TO_WRITE * 1000);
//     terminal.println("Average write time: " + String(avg_write_time, 2) + "mS");

//     long bytes_written = 512 * BLOCKS_TO_WRITE;
//     terminal.println("Number of bytes written: " + String(bytes_written) + " bytes");

//     double avg_write_speed = (512 * BLOCKS_TO_WRITE) / avg_write_time;
//     terminal.println("Average write speed: " + String((avg_write_speed / 1000000), 2) + " MB/s");

//     esp.uart0.println();
// }

#define BLOCKS_TO_WRITE 1000 // Blocks of 512 bytes

// void LoggerData::toCSVLine()
// {
//     long initial_time = micros();
//     char data[512];
//     uint16_t length = 0;

//     time_stamp = SoM.system_time;

//     //* 1. Time stamp (27)
//     char timestamp_str[27];
//     length += sprintf(timestamp_str, "%4i-%02i-%02iT%02i:%02i:%02i.%03i%03iZ,",
//                       time_stamp.year,
//                       time_stamp.month,
//                       time_stamp.day,
//                       time_stamp.hours,
//                       time_stamp.minutes,
//                       time_stamp.seconds,
//                       time_stamp.milliseconds,
//                       time_stamp.microseconds);

//     strcpy(data, timestamp_str);

//     char analog_ch_str[(6 * NUMBER_OF_CHANNELS) + NUMBER_OF_CHANNELS];
//     // length += sprintf(analog_ch_str, "%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf,%2.3lf",
//     //                   analog_channels[0],
//     //                   analog_channels[1],
//     //                   analog_channels[2],
//     //                   analog_channels[3],
//     //                   analog_channels[4],
//     //                   analog_channels[5],
//     //                   analog_channels[6],
//     //                   analog_channels[7]);

//     // strcat(data, analog_ch_str);

//     char raw_analog_ch_str[(6 * NUMBER_OF_CHANNELS) + NUMBER_OF_CHANNELS];
//     length += sprintf(raw_analog_ch_str, "%6i,%6i,%6i,%6i,%6i,%6i,%6i,%6i",
//                       raw_analog_channels[0],
//                       raw_analog_channels[1],
//                       raw_analog_channels[2],
//                       raw_analog_channels[3],
//                       raw_analog_channels[4],
//                       raw_analog_channels[5],
//                       raw_analog_channels[6],
//                       raw_analog_channels[7]);

//     strcat(data, raw_analog_ch_str);

//     // strcat(data, "\n");

//     // for (int i = 0; i < 4; i++)
//     // {
//     //     strcat(data, timestamp_str);
//     //     strcat(data, analog_ch_str);
//     //     strcat(data, "\n");
//     // }

//     // strcat(data, timestamp_str);
//     // strcat(data, analog_ch_str);

//     terminal.printMessage(TerminalMessage(String(length), "MMC", INFO, micros(),
//                                           micros() - initial_time));

//     initial_time = micros();

//     emmc.appendFile(file_name, data);

//     terminal.printMessage(TerminalMessage("File saved", "MMC", INFO, micros(),
//                                           micros() - initial_time));

//     //* 2. distance;    // 11 (2)
//     //* 3. lap_time;  // 16
//     //* 4. lap_number; // 4

//     //* 5. satelites_used; // 2
//     //* 6.latitude;        // 10 (6)
//     //* 7. longitude;       // 10 (6)
//     //* 8.speed;           // 5 (2)
//     //* 9. heading;         // /?

//     //* 10.analog_channels[8]; // 48 ( 8 x 6)

//     //* 11. accel_x;   // 6
//     //* 12. accel_y;   // 6
//     //* 13. accel_z;   // 6
//     //* 14. yaw_rate;  // 6
//     //* 15. altittude; // 6

//     //* 16. battery_voltage;   // 4
//     //* 17. v3v3_rail_voltage; // 4
//     //* 18. v5v_rail_voltage;  // 4
// }