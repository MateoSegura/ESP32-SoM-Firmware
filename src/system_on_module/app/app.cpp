// /*
//  * File Name: soc_example.cpp
//  * Hardware needed: No extra hardware needed, just a good ol' ESP32 dev. board
//  *
//  * Copyright 2021, Mateo Segura, All rights reserved.
//  */

// #include <Arduino.h>
// #include <esp32_utilities.h>
// #include <ACAN2517FD.h>

// #define UART0_BAUD_RATE 115200         // Set the baudrate of the terminal uart port
// #define MICROS_TIMESTAMP_ENABLED false // Set to true to enabled microsecond time stamp in terminal messages
// #define SYSTEM_TIME_ENABLED false      // Set to true if using a real time clock. Refer to "rtc_example.cpp" for more

// #define GND_DEBUG_EXP_PIN 2     // Connect pin 0 of the expansion to GPIO 26 of the ESP
// #define IO_EXPANSION_INT_PIN 35 // Input only. Need's external pull up resistor
// #define SX1509_I2C_ADDR 0x3E    // ADDR0 & ADDR1 are set to 0

// #define I2C0_SDA_PIN 21
// #define I2C0_SCL_PIN 22
// #define I2C0_FREQUENCY 400000 // KHz

// #define ADC_CS_PIN 33
// #define VSPI_SDI_PIN 23
// #define VSPI_SDO_PIN 19
// #define VSPI_CLK_PIN 18
// #define VSPI_CLK_FREQUENCY 25000000

// #define MCP2517_SCK 16 // 16
// #define MCP2517_SDI 25 // 25
// #define MCP2517_SDO 17 // 17
// #define MCP2517_CS 32  // 30
// #define MCP2517_INT 36 // 36

// struct GPIO_Write
// {
//   uint8_t pin;
//   bool state;
// };

// SystemOnChip esp;
// Terminal terminal;
// SX1509 io_expansion;
// AD7689 adc;
// ACAN2517FD can(MCP2517_CS, esp.hspi, MCP2517_INT);

// void setup()
// {
//   // 1. Init UART0(Serial)
//   esp.uart0.begin(UART0_BAUD_RATE);

//   esp.uart0.println("\n\n"); // The esp.uart0 object is the "Serial" object from Arduino. Refer to "soc_example.cpp" for more
//   esp.uart0.println("************************************************************************************************************");
//   esp.uart0.println("*                                       System on Chip Example                                             *");
//   esp.uart0.println("************************************************************************************************************");
//   esp.uart0.println("\n\n");

//   // 2. Init Terminal
//   terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);

//   // 3. Init I2C bus
//   esp.i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_FREQUENCY); // Refer to soc_example.cpp for information on this function

//   // 4. Init I / O expansion
//   ESP_ERROR initialize_io_expansion = io_expansion.begin(&esp.i2c0, SX1509_I2C_ADDR);

//   if (initialize_io_expansion.on_error) // Catch error
//   {
//     while (1)
//     {
//       terminal.printMessage(TerminalMessage(initialize_io_expansion.debug_message, "I/O", ERROR, micros()));
//       delay(500);
//     }
//   }

//   terminal.printMessage(TerminalMessage("SX1509 External I/O initialized correctly", "I/O", ERROR, micros()));

//   // 5V
//   io_expansion.pinMode(9, OUTPUT);
//   io_expansion.digitalWrite(9, HIGH);

//   // gnd debug
//   io_expansion.pinMode(GND_DEBUG_EXP_PIN, OUTPUT);
//   io_expansion.digitalWrite(GND_DEBUG_EXP_PIN, LOW);

//   // 3.3v debug
//   io_expansion.pinMode(1, OUTPUT);
//   io_expansion.digitalWrite(1, HIGH);

//   // 3.3v out
//   io_expansion.pinMode(0, OUTPUT);
//   io_expansion.digitalWrite(0, HIGH);
//   io_expansion.pinMode(3, OUTPUT);
//   io_expansion.digitalWrite(3, HIGH);

//   // adc
//   io_expansion.pinMode(4, OUTPUT);
//   io_expansion.digitalWrite(4, HIGH);

//   io_expansion.pinMode(5, OUTPUT);
//   io_expansion.digitalWrite(5, LOW);

//   // 3. Begin SPI bus
//   esp.vspi.begin(VSPI_CLK_PIN, VSPI_SDO_PIN, VSPI_SDI_PIN);
//   esp.vspi.setFrequency(VSPI_CLK_FREQUENCY);

//   // 4. Begin ADC
//   adc.begin(ADC_CS_PIN, esp.vspi, VSPI_CLK_FREQUENCY);

//   if (adc.selftest() == false)
//   {
//     terminal.printMessage(TerminalMessage("Error initializing ADC", "ADC", ERROR, micros()));
//     while (1)
//       ;
//   }

//   terminal.printMessage(TerminalMessage("ADC initialized correctly", "ADC", ERROR, micros()));

//   // 3. Init HSPI bus
//   esp.hspi.begin(MCP2517_SCK, MCP2517_SDO, MCP2517_SDI);

//   // 4. Init CAN bus long initial_time = micros();
//   ACAN2517FDSettings settings(ACAN2517FDSettings::OSC_40MHz, 1000 * 1000, DataBitRateFactor::x1);

//   settings.mDriverReceiveFIFOSize = 200;

//   const uint32_t error_code = can.begin(settings, []
//                                         { can.isr(); });

//   if (error_code != 0)
//   {
//     while (1)
//     {
//       terminal.printMessage(TerminalMessage("Could not initialize CAN controller. Check connections",
//                                             "WIF", ERROR, micros()));
//       delay(500);
//     }
//   }

//   terminal.printMessage(TerminalMessage("CAN controller initialized", "WIF", ERROR, micros()));
// }

// //********************LOOP
// void loop()
// {
// }
