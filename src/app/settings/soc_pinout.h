#pragma once

/*
 * File Name: soc_pinout.h
 * Project: ESP32 System on Module
 */

//***************************** READ ME

//*  This file contains the pinout of the ESP32 and GPIO expansion
//*
//*  Baud rates & bus clock frequencies are in "settings/soc_clk_freq.h"

//********************************************************     ESP32 PINS    ********************************************************/

//* UART0
#define UART0_TX 1
#define UART0_RX 3

//* I2C0
#define IO_EXP_ADDRESS 0x3E
//  0x52 -> Real Time Clock
#define I2C0_SDA_PIN 21
#define I2C0_SCL_PIN 22

//* I2C1
//  0x68 -> Internal Motion Unit
//  0x76 -> Env. sensor
#define I2C1_SDA_PIN 4
#define I2C1_SCL_PIN 5

//* HSPI
#define HSPI_MOSI_PIN 25
#define HSPI_MISO_PIN 17
#define HSPI_CLCK_PIN 16

//* VSPI
#define VSPI_MOSI_PIN 23
#define VSPI_MISO_PIN 19
#define VSPI_CLCK_PIN 18

//* Analog to Digital Converter
#define ADC0_CS 33 // AD7689A is connected to the VSPI bus

//* CAN Controller
#define CAN0_CONTROLLER_CS_PIN 32  // MCP2518 is connected to the HSPI bus
#define CAN0_CONTROLLER_INT_PIN 36 // Active Low. Pull up resistor on module

//* Real Time Clock
#define RTC0_INT 39 // Active Low. Pull up resistor on module

//* Internal Motion Unit
#define IMU0_INT 13 // Populate R12 & remove R11 to enable

//* GPIO Expansion
#define IO_EXPANSION_INT_PIN 35 // Active Low. Pull up resistor on module
// #define IO_EXPANSION_RST_PIN 13 // Populate R11 & remove R12 to enable

//* Neopixel
#define NEOPIXEL_PIN 0

//* User internal I/O
#define GPIO12 12 // Input, Output. DO NOT PULL HIGH. Boot fail if high after reset
#define GPIO13 13 // Input, Output
#define GPIO26 26 // Input, Output66
#define GPIO27 27 // Input, Output
#define GPIO34 34 // Input Only

//******************************************************     GPIO EXP. PINS    *******************************************************/

//* +3.3V Out
#define V3V3_OUT_PIN_A 0
#define V3V3_OUT_PIN_B 3

//* +3.3V Debug
#define V3V3_DEBUG_PIN 1

//* GND Debug
#define GND_DEBUG_PIN 2

//* System Enable Pins
#define ADC0_EN_PIN 4  // ADC. Active high
#define CAN0_EN_PIN 5  // CAN. Active low
#define eMMC0_EN_PIN 6 // eMMC. Active low
#define eMMC0_CD_PIN 7 // eMMC Card detect
#define IMU_EN_PIN 8   // IMU. Ative low
#define V5V_EN_PIN 9   // +5V. Active high

//* User external I/O
#define EXP_GPIO10 10
#define EXP_GPIO11 11
#define EXP_GPIO12 12
#define EXP_GPIO13 13
#define EXP_GPIO14 14
#define EXP_GPIO15 15

//* EMMC Memory

//* End.