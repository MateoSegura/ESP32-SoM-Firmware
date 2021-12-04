#include "app.h"
#include "utils/utils.h"

SystemOnModule SoM;
DataLogger logger;

void i2cTest();

void Application::begin()
{
    // 1. Begin esp.uart0 port for debugging
    esp.uart0.begin(UART0_BAUD_RATE);
    terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);
    terminal.printBanner("Data Logging Application");

    // 2. Init System on Module
    SoM.initAll(DEBUGGING_ENABLED);

    // 3. Register RTOS variables
    initRTOS();

    // 4. Init Carrier Board hardware
    ESP_ERROR init_gps = initGPS();

    if (init_gps.on_error)
        handleError(init_gps.debug_message, "APP");

    TerminalMessage message = TerminalMessage("GPS Initialized", "APP", INFO, micros());
    printMessage(message);

    // 5. Init terminal task used by other tasks
    xTaskCreatePinnedToCore(terminalOutput, "Terminal", 10000, nullptr, 1, nullptr, 0);

    // 6. Init Logger
    DataLoggerSettings logger_settings;

    logger_settings.frequency = DataLoggerSettings::_50_Hz;

    logger.begin(logger_settings);
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
