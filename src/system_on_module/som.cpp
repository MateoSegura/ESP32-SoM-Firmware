// #include "som.h"
// #include <ACAN2517FD.h>

// //* ESP32
// SystemOnChip esp;
// Terminal terminal;
// SPIFFS_Memory spiffs;
// EMMC_Memory emmc;
// // BluetoothLowEnergyServer bleServer;

// //* Peripherals
// SX1509 ioExpansion;
// RealTimeClock rtc;
// Adafruit_NeoPixel ledStrip(0, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// AD7689 adc;
// ACAN2517FD can(CAN0_CONTROLLER_CS_PIN, esp.hspi, CAN0_CONTROLLER_INT_PIN);

// ESP_ERROR ESP32_SystemOnModule::begin(bool enable_debugging)
// {
//     ESP_ERROR err;
//     long initial_time;
//     debugging = enable_debugging;

//     // 1. Terminal
//     esp.uart0.begin(UART0_BAUD_RATE);
//     terminal.begin(esp.uart0);
//     if (debugging)
//         terminal.printMessage(TerminalMessage("Boot started", "   ", INFO, micros()));

//     // 2. Init I/O expansion
//     initial_time = micros();
//     ESP_ERROR init_external_io = initIO_Expansion();

//     if (init_external_io.on_error)
//     {
//         while (1)
//         {
//             terminal.printMessage(TerminalMessage(init_external_io.debug_message, "   ", ERROR, micros()));
//             delay(1000);
//         }
//     }

//     if (debugging)
//         terminal.printMessage(TerminalMessage("I/O expansion initialized correctly", "   ", INFO, micros(),
//                                               micros() - initial_time));

//     // 3. Init ADC
//     // initial_time = micros();
//     // ESP_ERROR init_adc = initADC();

//     // if (init_adc.on_error)
//     // {
//     //     while (1)
//     //     {
//     //         terminal.printMessage(TerminalMessage(init_adc.debug_message, "   ", ERROR, micros()));
//     //         delay(1000);
//     //     }
//     // }

//     // if (debugging)
//     //     terminal.printMessage(TerminalMessage("ADC initialized correctly", "   ", INFO, micros(),
//     //                                           micros() - initial_time));

//     // 4. CAN bus
// }

// ESP_ERROR ESP32_SystemOnModule::initIO_Expansion()
// {
//     ESP_ERROR err;

//     long initial_time = micros();
//     // 4. Init I / O expansion
//     esp.i2c0.begin(I2C0_SDA, I2C0_SCL, I2C0_CLK_FREQUENCY);
//     ESP_ERROR initialize_io_expansion = ioExpansion.begin(&esp.i2c0, IO_EXP_ADDRESS);

//     if (initialize_io_expansion.on_error) // Catch error
//     {
//         while (1)
//         {
//             terminal.printMessage(TerminalMessage(initialize_io_expansion.debug_message, "I/O", ERROR, micros()));
//             delay(500);
//         }
//     }

//     terminal.printMessage(TerminalMessage("SX1509 External I/O initialized correctly", "I/O", ERROR, micros()));

//     // 5v
//     ioExpansion.pinMode(9, OUTPUT);
//     ioExpansion.digitalWrite(9, HIGH);

//     // gnd debug
//     ioExpansion.pinMode(GND_DEBUG_PIN, OUTPUT);
//     ioExpansion.digitalWrite(GND_DEBUG_PIN, LOW);

//     // 3.3v debug
//     ioExpansion.pinMode(1, OUTPUT);
//     ioExpansion.digitalWrite(1, HIGH);

//     // 3.3v out
//     ioExpansion.pinMode(0, OUTPUT);
//     ioExpansion.digitalWrite(0, HIGH);
//     ioExpansion.pinMode(3, OUTPUT);
//     ioExpansion.digitalWrite(3, HIGH);

//     // adc
//     ioExpansion.pinMode(4, OUTPUT);
//     ioExpansion.digitalWrite(4, HIGH);

//     ioExpansion.pinMode(5, OUTPUT);
//     ioExpansion.digitalWrite(5, LOW);

//     vTaskDelay(200 / portTICK_PERIOD_MS);

//     // 3. Begin SPI bus
//     esp.vspi.begin(VSPI_CLCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
//     esp.vspi.setFrequency(VSPI_CLCK_FREQUENCY);

//     // 4. Begin ADC
//     adc.begin(ADC0_CS, esp.vspi, VSPI_CLCK_FREQUENCY);

//     if (adc.selftest() == false)
//     {
//         terminal.printMessage(TerminalMessage("Error initializing ADC", "ADC", ERROR, micros()));
//         while (1)
//             ;
//     }

//     terminal.printMessage(TerminalMessage("ADC initialized correctly", "ADC", ERROR, micros()));

//     // 3. Init HSPI bus
//     esp.hspi.begin(HSPI_CLCK_PIN, HSPI_MISO_PIN, HSPI_MOSI_PIN);

//     // 4. Init CAN bus long initial_time = micros();
//     ACAN2517FDSettings settings(ACAN2517FDSettings::OSC_40MHz, 1000 * 1000, DataBitRateFactor::x1);

//     settings.mDriverReceiveFIFOSize = 200;

//     const uint32_t error_code = can.begin(settings, []
//                                           { can.isr(); });

//     if (error_code != 0)
//     {
//         while (1)
//         {
//             terminal.printMessage(TerminalMessage("Could not initialize CAN controller. Check connections",
//                                                   "WIF", ERROR, micros()));
//             delay(500);
//         }
//     }

//     terminal.printMessage(TerminalMessage("CAN controller initialized", "WIF", ERROR, micros(),
//                                           micros() - initial_time));

//     // ESP_ERROR err;

//     // // 1. Init I2C bus & IO expansion
//     // esp.i2c0.begin(I2C0_SDA, I2C0_SCL, I2C0_CLK_FREQUENCY);
//     // ESP_ERROR init_io_exp = ioExpansion.begin(&esp.i2c0, IO_EXP_ADDRESS);

//     // if (init_io_exp.on_error)
//     // {
//     //     err.on_error = true;
//     //     err.debug_message = "Could not initialize SX1509";
//     //     return err;
//     // }

//     // // // 2. Setup inputs
//     // // ioExpansion.pinMode(eMMC0_CD_PIN, INPUT);

//     // // // 3 Setup debug outputs
//     // // ioExpansion.pinMode(V3V3_OUT_PIN_A, OUTPUT);
//     // // ioExpansion.pinMode(V3V3_OUT_PIN_B, OUTPUT);
//     // // ioExpansion.pinMode(V3V3_DEBUG_PIN, OUTPUT);
//     // // ioExpansion.pinMode(GND_DEBUG_PIN, OUTPUT);

//     // // if (debugging)
//     // // {
//     // //     ioExpansion.digitalWrite(V3V3_DEBUG_PIN, HIGH);
//     // //     ioExpansion.digitalWrite(GND_DEBUG_PIN, LOW);
//     // // }
//     // // else
//     // // {
//     // //     ioExpansion.digitalWrite(V3V3_DEBUG_PIN, LOW);
//     // //     ioExpansion.digitalWrite(GND_DEBUG_PIN, HIGH);
//     // // }

//     // // // 4. 5V systems
//     // // ioExpansion.pinMode(V5V_EN_PIN, OUTPUT);
//     // // ioExpansion.digitalWrite(V5V_EN_PIN, HIGH);

//     // // // 5. Turn off eMMC
//     // // // pinMode(14, OUTPUT);
//     // // // pinMode(15, OUTPUT);
//     // // // pinMode(13, OUTPUT);

//     // // // digitalWrite(13, LOW);
//     // // // digitalWrite(14, LOW);
//     // // // digitalWrite(15, LOW);

//     // // // 5. ADC
//     // // ioExpansion.pinMode(ADC0_EN_PIN, OUTPUT);
//     // // ioExpansion.digitalWrite(ADC0_EN_PIN, HIGH);

//     // // v5v_enabled = true;

//     // // 5v
//     // ioExpansion.pinMode(9, OUTPUT);
//     // ioExpansion.digitalWrite(9, HIGH);

//     // // gnd debug
//     // ioExpansion.pinMode(GND_DEBUG_PIN, OUTPUT);
//     // ioExpansion.digitalWrite(GND_DEBUG_PIN, LOW);

//     // // 3.3v debug
//     // ioExpansion.pinMode(1, OUTPUT);
//     // ioExpansion.digitalWrite(1, HIGH);

//     // // 3.3v out
//     // ioExpansion.pinMode(0, OUTPUT);
//     // ioExpansion.digitalWrite(0, HIGH);
//     // ioExpansion.pinMode(3, OUTPUT);
//     // ioExpansion.digitalWrite(3, HIGH);

//     // // adc
//     // ioExpansion.pinMode(4, OUTPUT);
//     // ioExpansion.digitalWrite(4, HIGH);

//     // ioExpansion.pinMode(5, OUTPUT);
//     // ioExpansion.digitalWrite(5, LOW);

//     // vTaskDelay(200 / portTICK_PERIOD_MS);

//     return err;
// }

// ESP_ERROR ESP32_SystemOnModule::initADC()
// {
//     // ESP_ERROR err;

//     // // 1. Enable ADC

//     // // 2. Begin VSPI bus
//     // esp.vspi.begin(VSPI_CLCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
//     // esp.vspi.setFrequency(VSPI_CLCK_FREQUENCY);

//     // // 3. Init ADC
//     // adc.begin(ADC0_CS, esp.vspi, VSPI_CLCK_FREQUENCY);

//     // // // 4. Run self test
//     // // if (adc.selftest() == false)
//     // // {
//     // //     err.on_error = true;
//     // //     err.debug_message = "Could not initialize ADC";
//     // // }

//     // return err;
// }
