#include "som.h"

SystemOnChip esp;
Terminal terminal;
EMMC_Memory emmc;

RealTimeClock rtc;
SX1509 io_expansion;
AD7689 adc;
ACAN2517FD can(CAN0_CONTROLLER_CS_PIN, esp.hspi, CAN0_CONTROLLER_INT_PIN);
Adafruit_NeoPixel pixels(2, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// MPU9250 imu(0x68, esp.i2c1, I2C1_CLK_FREQUENCY);
Adafruit_BME680 bme(&esp.i2c1);
SFE_UBLOX_GPS gps;

void IRAM_ATTR updateSystemTime()
{
    rtc.updateMillisecondsCounter(SoM.system_time);
}

void SystemOnModule::onBootError()
{
    SoM.initLED();
    pixels.setPixelColor(0, pixels.Color(100, 0, 0));
    pixels.show();
    terminal.println("\n\nStopping boot");
    while (1)
        ;
}

void SystemOnModule::initTimers()
{
    esp.timer0.setup();
    esp.timer0.attachInterrupt(updateSystemTime);
    esp.timer0.timerPeriodMilliseconds(1);
    esp.timer0.enableInterrupt();
}

void SystemOnModule::initAll(bool debug_enabled)
{
    long initial_time = millis();

    if (debug_enabled)
        debugging_enabled = true;

    if (!initIOExpansion())
        onBootError();

    initLED();

    if (!initRTC())
        onBootError();

    if (!initADC())
        onBootError();

    if (!initCAN())
        onBootError();

    if (!initMMC())
        onBootError();

    // if (!initIMU())
    //     onBootError();

    // if (!initBME())
    //     onBootError();

    // TODO: Init Env. sensor

    // initTimers();

    terminal.printMessage(TerminalMessage("Boot time: " + String(millis() - initial_time) + " mS", "SOM", INFO, micros()));
}

bool SystemOnModule::initIOExpansion()
{
    // 1. Begin I2C bus
    esp.i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_CLK_FREQUENCY); // Refer to soc_example.cpp for information on this function

    // 2. Begin IO expansion SX1509
    ESP_ERROR initialize_io_expansion = io_expansion.begin(&esp.i2c0, IO_EXP_ADDRESS);

    if (initialize_io_expansion.on_error) // Catch error
    {
        terminal.printMessage(TerminalMessage(initialize_io_expansion.debug_message, "I/O", ERROR, micros()));
        return false;
    }

    io_expansion.pinMode(V3V3_OUT_PIN_A, OUTPUT);
    io_expansion.digitalWrite(V3V3_OUT_PIN_A, HIGH);
    io_expansion.pinMode(V3V3_OUT_PIN_B, OUTPUT);
    io_expansion.digitalWrite(V3V3_OUT_PIN_B, HIGH); // 3.3V Out

    // 3. Setup GPIOs
    io_expansion.pinMode(V5V_EN_PIN, OUTPUT);
    io_expansion.digitalWrite(V5V_EN_PIN, HIGH); // 5V Output

    if (debugging_enabled)
    {
        io_expansion.pinMode(V3V3_DEBUG_PIN, OUTPUT);
        io_expansion.pinMode(GND_DEBUG_PIN, OUTPUT);
        io_expansion.digitalWrite(GND_DEBUG_PIN, LOW);   // GND Debug
        io_expansion.digitalWrite(V3V3_DEBUG_PIN, HIGH); // 3.3V Debug
    }

    io_expansion.pinMode(ADC0_EN_PIN, OUTPUT);
    io_expansion.digitalWrite(ADC0_EN_PIN, HIGH); // ADC

    io_expansion.pinMode(CAN0_EN_PIN, OUTPUT);
    io_expansion.digitalWrite(CAN0_EN_PIN, LOW); // CAN

    io_expansion.pinMode(IMU_EN_PIN, OUTPUT);
    io_expansion.digitalWrite(IMU_EN_PIN, LOW); // CAN

    terminal.printMessage(TerminalMessage("All systems enabled", "I/O", INFO, micros()));

    return true;
}

bool SystemOnModule::initRTC()
{
    long initial_time = micros();

    ESP_ERROR initialize_rtc = rtc.begin(RealTimeClock::RV3028_IC, esp.i2c0);

    if (initialize_rtc.on_error)
    {
        terminal.printMessage(TerminalMessage(initialize_rtc.debug_message, "RTC", ERROR, micros()));
        return false;
    }

    if (debugging_enabled)
    {
        terminal.printMessage(TerminalMessage("RTC initialized", "RTC", INFO, micros(),
                                              micros() - initial_time));
    }

    // rtc.setToCompilerTime();

    system_time = rtc.getTime();

    terminal.setTimeKeeper(system_time);

    return true;
}

bool SystemOnModule::initADC()
{
    long initial_time = micros();

    // 1. Begin SPI bus
    esp.vspi.begin(VSPI_CLCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
    esp.vspi.setFrequency(VSPI_CLCK_FREQUENCY);

    // 2. Begin ADC
    adc.begin(ADC0_CS, esp.vspi, VSPI_CLCK_FREQUENCY);
    // adc.enableFiltering(true);

    // 3. Run self test
    if (adc.selftest() == false)
    {
        terminal.printMessage(TerminalMessage("Error initializing ADC", "ADC", ERROR, micros()));
        return false;
    }

    if (debugging_enabled)
    {
        terminal.printMessage(TerminalMessage("ADC initialized", "ADC", INFO, micros(),
                                              micros() - initial_time));
    }

    return true;
}

bool SystemOnModule::initCAN()
{
    long initial_time = micros();

    // 1. Init SPI bus
    esp.hspi.begin(HSPI_CLCK_PIN, HSPI_MISO_PIN, HSPI_MOSI_PIN);

    // 2. Set CAN settings
    ACAN2517FDSettings settings(ACAN2517FDSettings::OSC_40MHz, 1000 * 1000, DataBitRateFactor::x1);
    settings.mDriverReceiveFIFOSize = 200;

    // 3. Attempt to initialize
    const uint32_t error_code = can.begin(settings, []
                                          { can.isr(); });

    if (error_code != 0)
    {
        terminal.printMessage(TerminalMessage("Could not initialize CAN controller. Error code: " + String(error_code),
                                              "CAN", ERROR, micros()));
        return false;
    }

    if (debugging_enabled)
    {
        terminal.printMessage(TerminalMessage("CAN controller initialized", "CAN", INFO, micros(),
                                              micros() - initial_time));
    }

    return true;
}

// bool SystemOnModule::initIMU()
// {
//     long initial_time = micros();

//     esp.i2c1.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, I2C1_CLK_FREQUENCY);

//     byte readback = imu.readByte(0x68, WHO_AM_I_MPU9250);

//     if (readback != 0x71)
//     {
//         terminal.printMessage(TerminalMessage("Error. Data should be : " + String(0x71, HEX) +
//                                                   ",data read: " + String(readback, HEX),
//                                               "IMU", ERROR, micros()));

//         return false;
//     }

//     imu.MPU9250SelfTest(imu.selfTest);
//     imu.calibrateMPU9250(imu.gyroBias, imu.accelBias);
//     imu.initMPU9250();

//     // Magnemometer
//     readback = imu.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);

//     if (readback != 0x48)
//     {
//         terminal.printMessage(TerminalMessage("Could not initialize magnemometer. Data should be : " +
//                                                   String(0x71, HEX) +
//                                                   ",data read: " + String(readback, HEX),
//                                               "CAN", ERROR, micros()));

//         return false;
//     }

//     imu.initAK8963(imu.factoryMagCalibration);
//     imu.getAres();
//     imu.getGres();
//     imu.getMres();

//     if (debugging_enabled)
//     {
//         terminal.printMessage(TerminalMessage("IMU initialized", "IMU", INFO, micros(),
//                                               micros() - initial_time));
//     }

//     // if (!imu.enabledReadbackInt())
//     // {
//     //     terminal.printMessage(TerminalMessage("Could not set readback interrupt", "IMU", INFO, micros(),
//     //                                           micros() - initial_time));
//     //     return false;
//     // }

//     // esp.uart0.print(F("x-axis self test: acceleration trim within : "));
//     // esp.uart0.print(imu.selfTest[0], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print(F("y-axis self test: acceleration trim within : "));
//     // esp.uart0.print(imu.selfTest[1], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print(F("z-axis self test: acceleration trim within : "));
//     // esp.uart0.print(imu.selfTest[2], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print(F("x-axis self test: gyration trim within : "));
//     // esp.uart0.print(imu.selfTest[3], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print(F("y-axis self test: gyration trim within : "));
//     // esp.uart0.print(imu.selfTest[4], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print(F("z-axis self test: gyration trim within : "));
//     // esp.uart0.print(imu.selfTest[5], 1);
//     // esp.uart0.println("% of factory value");
//     // esp.uart0.print("X-Axis factory sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[0], 2);
//     // esp.uart0.print("Y-Axis factory sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[1], 2);
//     // esp.uart0.print("Z-Axis factory sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[2], 2);
//     // esp.uart0.println("AK8963 mag biases (mG)");
//     // esp.uart0.println(imu.magBias[0]);
//     // esp.uart0.println(imu.magBias[1]);
//     // esp.uart0.println(imu.magBias[2]);
//     // esp.uart0.println("AK8963 mag scale (mG)");
//     // esp.uart0.println(imu.magScale[0]);
//     // esp.uart0.println(imu.magScale[1]);
//     // esp.uart0.println(imu.magScale[2]);
//     // esp.uart0.println("Magnetometer:");
//     // esp.uart0.print("X-Axis sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[0], 2);
//     // esp.uart0.print("Y-Axis sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[1], 2);
//     // esp.uart0.print("Z-Axis sensitivity adjustment value ");
//     // esp.uart0.println(imu.factoryMagCalibration[2], 2);

//     return true;
// }

bool SystemOnModule::initBME()
{
    long initial_time = micros();

    if (!bme.begin())
    {
        terminal.printMessage(TerminalMessage("Could not initialize BME688",
                                              "IMU", ERROR, micros()));

        return false;
    }

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    terminal.printMessage(TerminalMessage("BME initialized",
                                          "IMU", ERROR, micros(), micros() - initial_time));

    return true;
}

bool SystemOnModule::initMMC()
{
    long initial_time = micros();
    ESP_ERROR initialize_emmc = emmc.begin(-1, -1, eMMC_MODE, MODE_1_BIT);

    if (initialize_emmc.on_error)
    {
        terminal.printMessage(TerminalMessage(initialize_emmc.debug_message, "MMC", ERROR, micros()));
        return false;
    }

    if (debugging_enabled)
    {
        terminal.printMessage(TerminalMessage("eMMC initialized", "MMC", INFO, micros(),
                                              micros() - initial_time)); // How long did SD card take to init
    }

    return true;
}

void SystemOnModule::initLED()
{
    pixels.begin();
    pixels.clear();
    pixels.show();
}
